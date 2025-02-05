#include "stdafx.h"
#include "Programmer.h"
#include "PNPID.h"
#include "I2C_FTDI_D2XX.h"

Programmer::Programmer()
{
	i2c = new I2C_FTDI_D2XX();
	busyOpen = 0;
	closeCounter = 0;
	validEDID = false;
	currentFilename = NULL;
	cableready = false;
	connectError = NULL;
	reading = false;
	readindex = 0;
	writing = false;
	writeindex = 0;
	firstStart = true;
	silentError = false;
	EDIDdescription[0] = 0;
}


Programmer::~Programmer()
{
	disconnectCable();
	delete i2c;
}

bool Programmer::connectCable()
{
	if (cableready) return true;
	int err = i2c->open();
	if (err)
	{
		connectError = i2c->get_last_error();
		return false;
	}
	connectError = NULL;
	cableready = true;
	return cableready;
}

void Programmer::disconnectCable()
{
	if (cableready)
	{
		i2c->close();
		cableready = false;
	}
}

int Programmer::readBlock(unsigned int address, unsigned char* block)
{
	unsigned int transf = 0;
	unsigned char addr = address & 0xFF;
	int status = i2c->write(0x50, 1, &addr, &transf, I2C_TRANSFER_OPTIONS_START_BIT);
	if (status != RET_OK)
	{
		if (status == RET_DEVICE_ERROR)
		{
			connectError = u8"Écran non détecté.";
			return ERROR_DEVICE;
		}
		else
		{
			connectError = u8"Erreur de transmission.";
			return ERROR_CABLE;
		}
	}
	status = i2c->read(0x50, 16, block, &transf, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT | I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
	if (status != RET_OK)
	{
		connectError = u8"Erreur de transmission.";
		return ERROR_CABLE;
	}
	connectError = NULL;
	return transf;
}

int Programmer::writeBlock(unsigned int address, unsigned char* block)
{
	unsigned int transf = 0;
	unsigned char addr = address & 0xFF;
	unsigned char tbuf[17];
	tbuf[0] = address;
	for (int i = 0; i < 16; i++) tbuf[i + 1] = block[i];
	int status = i2c->write(0x50, 17, tbuf, &transf, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
#ifdef _WIN32
	Sleep(10);
#else
	usleep(10000);
#endif
	if (status != RET_OK)
	{
		if (status == RET_DEVICE_ERROR)
		{
			connectError = u8"Boîtier non détecté.";
			return ERROR_DEVICE;
		}
		else
		{
			connectError = u8"Erreur de transmission.";
			return ERROR_CABLE;
		}
	}
	//verify the data
	unsigned char check[16];
	int ret = readBlock(address, check);
	if (ret==16)
	{
		if (memcmp(check, block, 16))
		{
			//not the same data !
			connectError = u8"Echec de l'écriture, vérifier le bouton SEC.";
			return ERROR_WP;
		}
	}
	else
	{
		//error while reading back
		return ret;
	}
	connectError = NULL;
	return transf-1;
}

void Programmer::decodeEDID()
{
	//verify checksum
	unsigned char checksum = 0;
	for (int i = 0; i < 128; i++) checksum += EDID[i];
	if (checksum != 0)
	{
		connectError = u8"Somme de contrôle EDID invalide.";
		validEDID = false;
		return;
	}
	int numextensions = EDID[0x7E];
	if (numextensions > 0)
	{
		checksum = 0;
		for (int i = 128; i < 256; i++) checksum += EDID[i];
		if (checksum != 0)
		{
			connectError = u8"Somme de contrôle EDID invalide (extension 1).";
			validEDID = false;
			return;
		}
		if (numextensions > 1 || EDID[0xFE] > 0)
		{
			connectError = u8"Attention, certaines extensions de cette EDID ne sont pas supportées,\nelles ont été retirées.";
			//this is not a fatal error
			//patch the EDID to account for the first extension only
			EDID[0x7E] = 1;
			//recompute checksum
			checksum = 0;
			for (int i = 0; i < 127; i++) checksum += EDID[i];
			EDID[0x7F] = (256-checksum)&0xFF;
			//patch the remaining extension
			EDID[0xFE] = 0;
			//recompute checksum
			checksum = 0;
			for (int i = 128; i < 255; i++) checksum += EDID[i];
			EDID[0xFF] = (256 - checksum) & 0xFF;
		}
	}
	//Manufacturer
	char ISAcode[4];
	ISAcode[0] = ((EDID[0x08] >> 2) & 0x1F) + '@';
	ISAcode[1] = (((EDID[0x08] << 3) | (EDID[0x09] >> 5)) & 0x1F) + '@';
	ISAcode[2] = (EDID[0x09] & 0x1F) + '@';
	ISAcode[3] = 0;
	char* manuf = ISAcode;
	for (int i = 0; i < PNPID_SIZE; i++)
	{
		if (!strcmp(ISAcode, PNPID[i][1]))
		{
			manuf = PNPID[i][0];
			break;
		}
	}
	//screen size (diagonal in inches)
	float screensize = 0.0;
	if (EDID[0x15] != 0 && EDID[0x16] != 0)
	{
		//values in centimeters
		float x = EDID[0x15];
		float y = EDID[0x16];
		//we can calculate screen size
		screensize = sqrtf(x*x + y*y) / 2.54f;
	}
	//screen resolution
	int reswidth = (int)EDID[0x38] + ((int)(EDID[0x3A] >> 4) << 8);
	int resheight = (int)EDID[0x3B] + ((int)(EDID[0x3D] >> 4) << 8);
	//model name (if any)
	char modelname[14];
	modelname[0] = 0;
	for (int i = 0; i < 3; i++)
	{
		int base = 0x48 + i * 18;
		if (EDID[base] == 0 && EDID[base + 1] == 0 && EDID[base + 2] == 0 && EDID[base + 3] == 0xFC && EDID[base+4] == 0)
		{
			//we have a product name
			int j;
			for (j = 0; j < 13; j++)
			{
				if (EDID[base + 5 + j] <' ' || EDID[base + 5 + j]>127) break; //the standard says the character 0x0A is used to terminate the string but we are extra cautious
				modelname[j] = EDID[base + 5 + j];
			}
			modelname[j] = 0;
		}
	}
	sprintf(EDIDdescription, "%s (%s),  %.1f pouces, %ix%i.", manuf, modelname, screensize, reswidth, resheight);

}

bool Programmer::render(float scale, bool shouldClose)
{
	ImGuiIO& io = ImGui::GetIO();
	//main window
	if (ImGui::Begin("HDMI", NULL, ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoCollapse| ImGuiWindowFlags_NoSavedSettings))
	{
		//buttons states
		bool canreadfile = !reading && !writing;
		bool canwritefile = validEDID && !reading && !writing;
		bool canreadscreen = cableready &&!reading && !writing;
		bool canwritebox = cableready && validEDID && !reading && !writing;
		ImVec2 buttonsize(300.0f*scale, 100.0f*scale);
		//read file button
		if (!canreadfile)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button("Lire un fichier...\n\n    { }", buttonsize))
		{
			//read a file
			if (currentFilename) free(currentFilename);
			currentFilename = NULL;
			//validEDID = false;
			nfdresult_t result = NFD_OpenDialog("edid", NULL, &currentFilename);
			if (result == NFD_OKAY)
			{
				int len = MultiByteToWideChar(CP_UTF8, 0, currentFilename, -1, NULL, 0);
				wchar_t *wfn = (wchar_t*)malloc(sizeof(wchar_t)*len);
				MultiByteToWideChar(CP_UTF8, 0, currentFilename, -1, wfn, len);
				FILE* fd = _wfopen(wfn, L"rb");
				free(wfn);
				if (fd)
				{
					size_t ret = fread(EDID, 1, 256, fd);
					fclose(fd);
					if (ret == 256)
					{
						validEDID = true;
						decodeEDID();
					}
					else
					{
						validEDID = false;
						connectError = u8"Fichier invalide.";
					}
				}
				else
				{
					if (currentFilename) free(currentFilename);
					currentFilename = NULL;
					connectError = u8"Erreur à l'ouverture du fichier.";
				}
			}
		}
		if (!canreadfile)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::SameLine();
		//write file button
		if (!canwritefile)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button(u8"Enregistrer un fichier...\n\n        } {", buttonsize))
		{
			if (currentFilename) free(currentFilename);
			currentFilename = NULL;
			nfdresult_t result = NFD_SaveDialog("edid", NULL, &currentFilename);
			if (result == NFD_OKAY)
			{
				//add .edid to the end if necessary
				if (strlen(currentFilename)<5 || strcmp(".edid", currentFilename + (strlen(currentFilename) - 5)))
				{
					nfdchar_t *subs = (nfdchar_t*)malloc(sizeof(nfdchar_t)*(strlen(currentFilename) + 5));
					strcpy(subs, currentFilename);
					strcat(subs, ".edid");
					free(currentFilename);
					currentFilename = subs;
				}
				int len = MultiByteToWideChar(CP_UTF8, 0, currentFilename, -1, NULL, 0);
				wchar_t *wfn = (wchar_t*)malloc(sizeof(wchar_t)*len);
				MultiByteToWideChar(CP_UTF8, 0, currentFilename, -1, wfn, len);
				FILE* fd = _wfopen(wfn, L"wb");
				free(wfn);
				if (fd)
				{
					fwrite(EDID, 1, 256, fd);
					fclose(fd);
				}
				else
				{
					if (currentFilename) free(currentFilename);
					currentFilename = NULL;
				}
			}
		}
		if (!canwritefile)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		//read screen button
		if (!canreadscreen)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button(u8"Lire depuis un écran\n\n         ] }", buttonsize))
		{
			validEDID = false;
			reading = true;
			readindex = 0;
			if (currentFilename) free(currentFilename);
			currentFilename = NULL;
		}
		if (!canreadscreen)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::SameLine();
		//write box button
		if (!canwritebox)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button(u8"Écrire vers le boîtier\n\n      } [", buttonsize))
		{
			ImGui::OpenPopup(u8"Écriture mémoire");
		}
		//modal popup for write
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(1.0f, 0.5f, 0.0f, 0.35f));
		if (ImGui::BeginPopupModal(u8"Écriture mémoire", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(u8"ATTENTION : vérifiez que le câble n'est pas branché à un écran,");
			ImGui::Text(u8"car cette opération pourrait l'endommager.");
			ImGui::Text(u8"Le câble doit être branché au boîtier programmable (port \"ORDINATEUR\")");
			ImGui::Text(u8"et le commutateur SEC doit être en position OFF.");
			if (ImGui::Button(u8"Écrire"))
			{
				writing = true;
				writeindex = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"Annuler"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		if (!canwritebox)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		//silent error
		if (silentError && connectError)
		{
			silentError = false;
			connectError = NULL;
		}
		//error popup
		if (connectError)
		{
			ImGui::OpenPopup(u8"Erreur");
		}
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(1.0f, 0.0f, 0.0f, 0.35f));
		if (ImGui::BeginPopupModal(u8"Erreur", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(connectError);
			if (ImGui::Button(u8"OK"))
			{
				connectError = NULL;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		//find cable button
		if (!cableready)
		{
			if (ImGui::Button(u8"Rechercher un câble"))
			{
				busyOpen = 1;
				ImGui::OpenPopup(u8"Connection au câble");
			}
		}
		else
		{
			ImGui::Text(u8"Câble connecté (S/N : %s)", i2c->get_cable_serial());
		}
		//file name display
		if (currentFilename)
		{
			char* lastsep = currentFilename;
			for (int i = 0; i < strlen(currentFilename); i++) if (currentFilename[i] == '\\') lastsep = currentFilename + i + 1;
			ImGui::Text("Fichier : %s", lastsep);
		}
		//auto connect cable
		if (firstStart)
		{
			busyOpen = 1;
			ImGui::OpenPopup(u8"Connection au câble");
			//we don't want an error popup at startup
			silentError = true;
		}
		//connect cable busy popup
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0, 0.0, 0.0, 1.0));
		if (ImGui::BeginPopupModal(u8"Connection au câble", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(u8"Veuillez patienter, initialisation du câble...");
			if (busyOpen==0) ImGui::CloseCurrentPopup();
			busyOpen++;
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		if (busyOpen>=6) //hack to let the popup show up before we block the GUI
		{
			connectCable();
			busyOpen = 0;
		}
		//reading code
		if (reading)
		{
			int ret = readBlock(readindex * 16, EDID + readindex * 16);
			if (ret==16)
			{
				readindex++;
				if (readindex == NUMBLOCKS)
				{
					reading = false;
					validEDID = true;
					decodeEDID();
				}
			}
			else
			{
				reading = false;
				validEDID = false;
				if (ret==ERROR_CABLE) disconnectCable();
			}
		}
		//writing code
		if (writing)
		{
			int ret = writeBlock(writeindex * 16, EDID + writeindex * 16);
			if (ret==16)
			{
				writeindex++;
				if (writeindex == NUMBLOCKS)
				{
					writing = false;
					ImGui::OpenPopup(u8"Succès");
				}
			}
			else
			{
				writing = false;
				if (ret == ERROR_CABLE) disconnectCable();
			}
		}
		//success popup
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 1.0f, 0.0f, 0.35f));
		if (ImGui::BeginPopupModal(u8"Succès", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(u8"L'écriture a réussi.");
			ImGui::Text(u8"Le commutateur SEC doit être remis en position ON.");
			ImGui::Text(u8"Le commutateur M/A est à garder en position OFF,\n sauf en cas d'incompatibilité avérée.");
			ImGui::Text(u8"Le boîtier doit être ensuite fermé et scellé.");
			if (ImGui::Button(u8"OK"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		if (shouldClose)
		{
			if (!cableready)
			{
				ImGui::End();
				return true; //quick finish
			}
			closeCounter++;
		}
		if (closeCounter==1)
		{
			ImGui::OpenPopup(u8"Fermeture");
		}
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0, 0.0, 0.0, 1.0));
		if (ImGui::BeginPopupModal(u8"Fermeture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(u8"Veuillez patienter, fermeture du câble...");
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		if (closeCounter >= 5) //hack to let the popup show up before we block the GUI
		{
			disconnectCable();
			ImGui::End();
			return true; //terminate the application properly
		}
		//show EDID
		if (validEDID)
		{
			ImGui::Text(EDIDdescription);
		}
		ImGui::PushFont(io.Fonts->Fonts[2]);
		for (int i = 0; i < NUMBLOCKS; i++)
		{
			if (validEDID || (reading && i < readindex))
			{
				if (writing && writeindex == i) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
				ImGui::Text("%02X :", i * 16);
				for (int j = 0; j < 16; j++)
				{
					ImGui::SameLine();
					ImGui::Text(" %02X", EDID[i * 16 + j]);
				}
				if (writing && writeindex == i) ImGui::PopStyleColor();
			}
		}
		ImGui::PopFont();

	}
	ImGui::End();
	firstStart = false;
	return false;
}