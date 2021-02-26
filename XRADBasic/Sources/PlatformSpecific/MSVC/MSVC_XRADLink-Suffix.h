// file PC_XRADLink-Suffix.h
//--------------------------------------------------------------
#ifndef XRAD__File_PC_XRADLink_Suffix_h
#define XRAD__File_PC_XRADLink_Suffix_h
//--------------------------------------------------------------

#if _MSC_VER >= 1900 // MSVC2015+
	#ifdef XRAD_MSVC_EHA
		#define XRAD_Library_Suffix_EH "EHA"
	#else
		#define XRAD_Library_Suffix_EH ""
	#endif
	#ifdef XRAD_DEBUG
		#define XRAD_Library_Suffix_Debug "D"
	#else
		#define XRAD_Library_Suffix_Debug ""
	#endif
	#define XRAD_Library_Suffix XRAD_Library_Suffix_Debug XRAD_Library_Suffix_EH
#endif

//--------------------------------------------------------------
#endif // XRAD__File_PC_XRADLink_Suffix_h
