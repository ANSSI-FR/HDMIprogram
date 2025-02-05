#include "stdafx.h"
#include "Programmer.h"

//Nice icons. This is a Base85 STB-compressed TTF with icons from the free font entypo by Daniel Bruce
//Glyphs are :
// [ = square
// ] = computer screen
// { = file
// } = left arrow
static const char icons[1585 + 1] =
"7])#######N=6T.'/###[),##-`($#Q6>##kZn426?q)C@->>#a?'o/VNV=B7Xot#S*m<-ve6#5(.0%Ju]v;1B@uu#(,>>#0.d<BjM.R9g'l?-E@pV-TT$=(]GFE$+fG<-%e1p.<_[FH"
":eGb%,Nu3L7-CG)o6Q<Bq>]S.tG<`s>Z+q&0iR/G#MFS7B6wV%g2WA-qmnUC?E,ob_-)#P1G/2B@C=GHFheS%pm1/qm]B,3wdd>$$YO&#+ac8.(Jl>#KT*=()r#Z$;k$C#jF<A'/Jss'"
"alVP&L0v&#0dQ/LdCFgMTL6uu.P(Z#+lQS%WgJe$uIbA#(7HZ$]BWV$xOF&#BN89&-A###SQs-$C$%?$7#juLo'NZ#S<6&Mi@^rMBN(Po_^]S%%/5##nOSq)2Qxd<3P_-#jH`,#;Dj9`"
"':c'A)Th$'IG:;$>[5/$07C#$?GI&,G)(w-]5FGMm8#&#?=;mLDwCHM.6pfLYia,#Q`-0##1RkX7:L#vpd%1vcUVUM7dBHNMn%s$:)mN%QhR3k=#b#$hrES7NS$X.2lOrZ>'E>#wJJQ("
"%Q'%#pf_50fx@d)@+p_4sG=c4kHA8%^>)U2RhxILmh]c2P7.Z7O7r>7Y9bc2(>oO74w+F%A<*jL%7GcMH_H##Ze^]+kCLX:k=@8%sF#v,J#ID*sbNT/s0q+MC(LK2$VC_%1ctM(%O$q3"
"x9_+D57I>#x%%G(Io7##8J3$l34I>#'XDO+1#r`3%.^Y5G.$E1N#<[$L^7v5NT*39[@K41s:-`E$*hUdUuIfL0^suP.c68%'9on0Vh$##='YD#jcRI252:Z-gMqG*m3rd-jdu?KAArB#"
"g_-lLsk_&4'bi$P/f$S#QkTtu%H:E#Bqf_#U=5wp^eFX(SAt7K<RlA#fo:p.lAMqCOGFe-EdK@0;nHiLFN9uG$xVUm:CX9C*k0tB^_gA#$i''#P9ZlfA(dERxmFA#SwhC#$ITc)'X^G)"
"'wFtH#Z6gulOdAM*0df(I*t>[8E`hLmL*^-)%H9iK[YR*(HC_&FYi_&)KC_&Moi_&*NC_&T.j_&W@NP&;Cj_&8l4R*gej_&'r`i0M$k_&lE(,)?5#v#1:<p%YO(Q065m_&VOJM'a_n_&"
"(HC_&94o_&)KC_&G_o_&*NC_&U3p_&cN$)*C^p_&8l4R*$Kq_&RKGY>hoh_&]-3$#D/Ss9$/2eGnm`9C?G0H-l9]/G-A2eG,6jtBrbJM='DvlEJr1*Ns:ESC7;ZhFgLqk1xgN$P[+X#H"
"RMB@H/`wiC='oF-g$fFH&8vhF@hViFvoUEHM3FGH,C%12NOtR/<rY1F%67JC$xANCL>w1B%KG_&#t7;-T<XoIj-:m9od/F%tx4GDS>x]GvG(@'j5WiBD,q92ae3A=ZmL/D>'*F.coa-6"
"g>4A'EUw?0-7auGJ+=L5E?Yc2,a>G2t[esM3U]rHhtw(3kA]>HLVG'SDB]3FsUBe6VR?M96CH>H:%pcEfdm`FQ,/&GMK6,E:CxlB#j+j1QR-?H#-QY5+v4L#hY2;2mLkEe4ZMfLF=6##"
"(@V8&Dtuf$Z2###*J(v#.c_V$##########_VumqHl###";


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#ifdef _WIN32
//unicode version
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	
	SetProcessDPIAware(); //as a safety measure, but it really should be set in the manifest
	//we can use GetSystemMetrics() GetDeviceCaps() or GetDpiForMonitor()

	float scale = 1.0;
	//retrieve DPI info
	HDC dc = GetDC(NULL);
	if (dc)
	{
		int dpix = GetDeviceCaps(dc, LOGPIXELSX); //Win7 compatible
		ReleaseDC(NULL, dc);
		scale = (float)dpix / 96.0f;
	}
#else
int main(int, char**)
{
	float scale = 1.0;

#endif
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow((int)(624.0f*scale), (int)(600.0f*scale), "Programmateur HDMI", NULL, NULL);
	if (window == NULL)
		return 2;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync


	// Setup ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//disable .ini
	io.IniFilename = NULL;

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();

	// Setup style
	ImGui::StyleColorsLight();
	ImGui::GetStyle().FrameRounding = 3;
	ImGui::GetStyle().WindowRounding = 3;
	ImGui::GetStyle().ChildRounding = 3;
	ImGui::GetStyle().ScrollbarRounding = 2;
	ImGui::GetStyle().GrabRounding = 2;
	ImGui::GetStyle().FramePadding = ImVec2(8, 3);

	io.Fonts->AddFontDefault();
	//font 1
	//we make holes in the glyph range for icons
	static const ImWchar basic_ranges[] = { 0x0020, '[' - 1, '[' + 1, ']' - 1, ']' + 1, '{' - 1, '{' + 1, '}' - 1, '}' + 1, 0x00FF, 0 };
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 22.0f*scale, NULL, basic_ranges);
	//merge with icons
	static const ImWchar icons_ranges[] = { '[','[',']',']','{','{','}','}', 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	//this font is merged to font 1
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(icons, 32.0f*scale, &icons_config, icons_ranges);
	
	//font 2
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f*scale);

	io.FontDefault = io.Fonts->Fonts[1];
	//scale all gui
	ImGui::GetStyle().ScaleAllSizes(scale);

	ImVec4 clear_color = ImVec4(0.75, 0.75, 0.75, 1.0);

	//Creates programmer
	Programmer* prog = new Programmer();

	bool shouldClose = false;
	// Main loop
	while (!shouldClose)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//force full size window
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);

		//draw Programmer GUI
		shouldClose=prog->render(scale, glfwWindowShouldClose(window));
		
		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}
	//cleanup programmer
	delete prog;

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

