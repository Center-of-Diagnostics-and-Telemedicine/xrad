﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2/11/2019 5:37:36 PM
	\author Kovbas (kovbas)
*/
#ifndef dialogs_h__
#define dialogs_h__

#include <XRADDicom/XRADDicom.h>
#include <XRADGUI/XRAD.h>

XRAD_BEGIN

namespace Dicom
{
	unique_ptr<Dicom::datasource_t>	GetDicomDataSource();
	unique_ptr<Dicom::datasource_t>	GetDicomDataSourceFolder();
	unique_ptr<Dicom::datasource_t>	GetDicomDataSourcePACS();


	//layouts
	shared_ptr<DynamicDialog::ControlContainer> pacs_layout(wstring &address, wstring &port, wstring &pacs_ae_title, wstring &local_ae_title, wstring &port_local, e_request_t&);
	shared_ptr<DynamicDialog::ControlContainer> query_layout(shared_ptr<DynamicDialog::Button> &execute_button, wstring &studies);
	shared_ptr<DynamicDialog::ControlContainer> folder_selection_layout(wstring &path, const wstring &prompt = L"a folder");
	shared_ptr<DynamicDialog::ControlContainer> studies_identificators_layout(wstring &studies_numbers, study_id_type_t &study_id_type);

} //namespace Dicom

XRAD_END

#endif // dialogs_h__