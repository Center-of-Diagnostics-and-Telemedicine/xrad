/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2/11/2019 5:37:36 PM
	\author Kovbas (kovbas)
*/
#include "pre.h"
#include "dialogs.h"

XRAD_BEGIN

namespace Dicom
{
	//=============================================================================================================================
	//layouts


	shared_ptr<DynamicDialog::ControlContainer> folder_selection_layout(wstring &path, const wstring &prompt)
	{
		auto destination_layout = make_shared<DynamicDialog::ControlContainer>(prompt, DynamicDialog::Layout::Horizontal);

		auto destination_path_field = make_shared<DynamicDialog::ValueStringEdit>(L"Choose or Enter a path to " + prompt,
			SavedGUIValue(&path),
			DynamicDialog::Layout::Vertical);
		destination_layout->AddControl(destination_path_field);

		auto destination_layout_buttons = make_shared<DynamicDialog::ControlContainer>(DynamicDialog::Layout::Vertical);
		destination_layout_buttons->AddControl(make_shared<DynamicDialog::TextLabel>(L""));

		auto browse_button = make_shared<DynamicDialog::Button>(L"Browse...",
			[&prompt, destination_path_field]()
			{
				destination_path_field->SetValue(GetFolderNameRead(L"Choose " + prompt));
			});
		destination_layout_buttons->AddControl(browse_button);
		destination_layout_buttons->AddControl(make_shared<DynamicDialog::Stretch>());

		destination_layout->AddControl(destination_layout_buttons);
		return destination_layout;
	}

	shared_ptr<DynamicDialog::ControlContainer> query_layout(shared_ptr<DynamicDialog::Button> &execute_button, wstring &studies)
	{
		auto query_layout = make_shared<DynamicDialog::ControlContainer>(L"Query parameters", DynamicDialog::Layout::Vertical);
		query_layout->AddControl(make_shared<DynamicDialog::TextLabel>(L"-- Under construction. --"));
		/*
			fields for query params
		*/
		query_layout->AddControl(make_shared<DynamicDialog::ValueStringEdit>(L"Study(s) numbers",
				SavedGUIValue(&studies),
				DynamicDialog::Layout::Vertical));

		auto query_layout_buttons = make_shared<DynamicDialog::ControlContainer>(DynamicDialog::Layout::Horizontal);
		query_layout_buttons->AddControl(make_shared<DynamicDialog::Stretch>());
		query_layout_buttons->AddControl(execute_button);

		query_layout->AddControl(query_layout_buttons);
		return query_layout;
	}





	//=========================================================================================================================
	// functions
	unique_ptr<datasource_t>	GetDicomDataSourcePACS()
	{
		auto dialog = make_shared<DynamicDialog::Dialog>(L"PACS datasource");

		wstring address_pacs{ L"" }, port_pacs{ L"" }, ae_title_pacs{ L"" }, ae_title_local{ L"" }, port_local{ L"" }; e_request_t request_type{ e_request_t::cmove };
		dialog->AddControl(pacs_layout(address_pacs, port_pacs, ae_title_pacs, ae_title_local, port_local, request_type));

		auto button_layout = make_shared<DynamicDialog::ControlContainer>(DynamicDialog::Layout::Horizontal);
		button_layout->AddControl(make_shared<DynamicDialog::Stretch>());
		auto apply_button = make_shared<DynamicDialog::Button>(L"Apply", DynamicDialog::DialogResultCode::Accepted);
		button_layout->AddControl(apply_button);
		auto cancel_button = make_shared<DynamicDialog::Button>(L"Cancel", DynamicDialog::DialogResultCode::Rejected);
		button_layout->AddControl(cancel_button);
		dialog->AddControl(button_layout);

		while (true)
		{
			dialog->Show();

			if (dialog->GetResult() == apply_button.get())
			{
				return make_unique<datasource_pacs>(address_pacs, wcstol(port_pacs.c_str(), nullptr, 10), ae_title_pacs, ae_title_local, wcstol(port_local.c_str(), nullptr, 10), request_type);
			}

			if (dialog->GetResult() == cancel_button.get())
			{
				throw canceled_operation("Operation of choose and check PACS has been canceled");
			}
		}
		return nullptr;
	}

	unique_ptr<datasource_t>	GetDicomDataSourceFolder()
	{
		auto path = GetFolderNameRead(L"Get DICOM directory");
		bool	sub = YesOrNo("Analyze subdirectories?");
		return make_unique<datasource_folder>(path, sub);
	}

	unique_ptr<datasource_t>	GetDicomDataSource()
	{
		auto result = Decide(L"Choose a datasource", {MakeButton(L"PACS", make_fn(GetDicomDataSourcePACS)), MakeButton(L"Folder", make_fn(GetDicomDataSourceFolder))});
		return result();
	}

} //namespace Dicom

XRAD_END