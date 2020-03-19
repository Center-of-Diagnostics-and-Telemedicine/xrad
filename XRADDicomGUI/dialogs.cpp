/*!
	* \file dialogs.cpp
	* \date 2/11/2019 5:37:36 PM
	*
	* \author Kovbas (kovbas)
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#include "pre.h"
#include "dialogs.h"

#include <XRAD/GUI/DynamicDialog.h> //for DynamicDialog


XRAD_BEGIN

namespace Dicom
{
	//=============================================================================================================================
	//layouts
	shared_ptr<DynamicDialog::ControlContainer> pacs_layout(wstring &address_pacs_p, wstring &port_pacs_p, wstring &ae_title_pacs_p, wstring &ae_title_local_p, wstring &port_local_p, e_request_t &request_type_p)
	{
		ae_title_local_p = L"XRAD_SCU";
		port_local_p = L"104";
		auto pacs_layout = make_shared<DynamicDialog::ControlContainer>(L"PACS parameters and check server availability", DynamicDialog::Layout::Horizontal);

		auto pacs_address = make_shared<DynamicDialog::ValueStringEdit>(L"Address (IP or name)", SavedGUIValue(&address_pacs_p), DynamicDialog::Layout::Vertical);
		pacs_layout->AddControl(pacs_address);

		auto pacs_port = make_shared<DynamicDialog::ValueStringEdit>(L"Port", SavedGUIValue(&port_pacs_p), DynamicDialog::Layout::Vertical);
		pacs_layout->AddControl(pacs_port);

		auto pacs_aetitle = make_shared<DynamicDialog::ValueStringEdit>(L"PACS AE_Title", SavedGUIValue(&ae_title_pacs_p), DynamicDialog::Layout::Vertical);
		pacs_layout->AddControl(pacs_aetitle);

		auto local_aetitle = make_shared<DynamicDialog::ValueStringEdit>(L"Local AE_Title", SavedGUIValue(&ae_title_local_p), DynamicDialog::Layout::Vertical);
		pacs_layout->AddControl(local_aetitle);

		auto local_port = make_shared<DynamicDialog::ValueStringEdit>(L"Local Port", SavedGUIValue(&port_local_p), DynamicDialog::Layout::Vertical);
		pacs_layout->AddControl(local_port);

		//auto request_type = make_shared<DynamicDialog::RadioButtonContainer>(L"Request Type", SavedGUIValue(&request_type_p), DynamicDialog::Layout::Vertical);
		auto request_type = DynamicDialog::EnumRadioButtonChoice::Create(
			L"Request Type",
			vector<Button<e_request_t>>
		{
			MakeButton(L"C-GET", e_request_t::cget),
			MakeButton(L"C-MOVE", e_request_t::cmove)
		},
				SavedGUIValue(&request_type_p),
				DynamicDialog::Layout::Vertical
		);
		pacs_layout->AddControl(request_type);

		auto check_field_layout = make_shared<DynamicDialog::ControlContainer>(DynamicDialog::Layout::Vertical);
		check_field_layout->AddControl(make_shared<DynamicDialog::TextLabel>(L""));

		auto pacs_availability_check_button = make_shared<DynamicDialog::Button>(L"Check server availability",
			[pacs_address, pacs_port, pacs_aetitle, local_aetitle, local_port, request_type]()
		{
			Dicom::pacs_params_t pacs(pacs_address->Value(), wcstol(pacs_port->Value().c_str(), nullptr, 10), pacs_aetitle->Value(), local_aetitle->Value(), wcstol(local_port->Value().c_str(), nullptr, 10));
				wstring title = L"Test result";
				if (Dicom::checkPACSAccessibility(pacs))
					ShowString(title, L"Server\n" + pacs.print() + L"\nis available");
				else
					ShowString(title, L"Server\n" + pacs.print() + L"\nis NOT available");
            });
		check_field_layout->AddControl(pacs_availability_check_button);

		check_field_layout->AddControl(make_shared<DynamicDialog::Stretch>());

		pacs_layout->AddControl(check_field_layout);

		return pacs_layout;
	}

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


	shared_ptr<DynamicDialog::ControlContainer> studies_identificators_layout(wstring &studies_numbers, study_id_type_t &study_id_type)
	{
		study_id_type = study_id_type_t::accession_number;
		auto studies_list_layout = make_shared<DynamicDialog::ControlContainer>(L"Studies identificators", DynamicDialog::Layout::Vertical);

		auto study_type_choise_layout = make_shared<DynamicDialog::ControlContainer>(DynamicDialog::Layout::Horizontal);
		study_type_choise_layout->AddControl(DynamicDialog::EnumRadioButtonChoice::Create(
			L"Choose study identificator type",
			{
				MakeButton(L"Accession number", study_id_type_t::accession_number),
				MakeButton(L"Study ID", study_id_type_t::study_id),
				MakeButton(L"Study instance UID", study_id_type_t::study_instance_uid)
			},
			//&study_id_type,
			SavedGUIValue(&study_id_type),
			DynamicDialog::Layout::Horizontal));
		study_type_choise_layout->AddControl(make_shared<DynamicDialog::Stretch>());
		studies_list_layout->AddControl(study_type_choise_layout);

		studies_list_layout->CreateControl<DynamicDialog::ValueTextEdit>(L"Enter studies identificators to download them", SavedGUIValue(&studies_numbers));
		return studies_list_layout;
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