/****************************************************************************

 AboutDisplayTmats.h - A .NET 2005 class that implements the "about" dialog

 Copyright (c) 2008 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met:

   * Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may 
     be used to endorse or promote products derived from this software 
     without specific prior written permission.

 This software is provided by the copyright holders and contributors 
 "as is" and any express or implied warranties, including, but not 
 limited to, the implied warranties of merchantability and fitness for 
 a particular purpose are disclaimed. In no event shall the copyright 
 owner or contributors be liable for any direct, indirect, incidental, 
 special, exemplary, or consequential damages (including, but not 
 limited to, procurement of substitute goods or services; loss of use, 
 data, or profits; or business interruption) however caused and on any 
 theory of liability, whether in contract, strict liability, or tort 
 (including negligence or otherwise) arising in any way out of the use 
 of this software, even if advised of the possibility of such damage.

 ****************************************************************************/

#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Reflection;

using namespace Irig106DotNet;


namespace I106Input {

	/// <summary>
	/// Summary for about
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class AboutDisplayTmats : public System::Windows::Forms::Form
	{
	public:
		AboutDisplayTmats(void)
    		{
			InitializeComponent();

            Assembly     ^ ExeAssembly = Assembly::GetExecutingAssembly();
            AssemblyName ^ ExeName     = ExeAssembly->GetName();
            Version      ^ ExeVersion  = ExeName->Version;
            lblVersion->Text = String::Format(
                "Display TMATS\r\nVerson {0}.{1}.{2}\n\rCopyright 2010\n\ririg106.org",
                ExeVersion->Major, ExeVersion->Minor, ExeVersion->Build);

//            Assembly     ^ DllAssembly = Assembly::GetAssembly(
//            AssemblyName ^ DllName     = DllAssembly->GetName();
//            Version      ^ DllVersion  = DllName->Version;
//            lblVersion->Text = String::Format(
//                "Display TMATS\r\nVerson {0}.{1}.{2}\n\rCopyright 2010\n\ririg106.org",
//                ExeVersion->Major, ExeVersion->Minor, ExeVersion->Build);

/*
            String  ^ LibDateTime;

            Irig106DotNet::Irig106Lib::DotNetLibVersion(iMajor, iMinor, iBuild);
            LibDateTime = Irig106DotNet::Irig106Lib::DotNetLibDateTime();
            DotNetLibVer->Text = String::Format("Irig 106 .NET Lib\n\rVer. {0}.{1}.{2}\n\r{3}", 
                iMajor, iMinor, iBuild, LibDateTime);
*/
	    	}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~AboutDisplayTmats()
		{
			if (components)
			{
				delete components;
			}
		}

    private: System::Windows::Forms::PictureBox^    pictureLogo;

    private: System::Windows::Forms::Button^        buttonClose;
    private: System::Windows::Forms::LinkLabel^     linkIrig106;
    private: System::Windows::Forms::Label^  lblVersion;

    private: System::Windows::Forms::Label^         DotNetLibVer;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(AboutDisplayTmats::typeid));
            this->pictureLogo = (gcnew System::Windows::Forms::PictureBox());
            this->buttonClose = (gcnew System::Windows::Forms::Button());
            this->linkIrig106 = (gcnew System::Windows::Forms::LinkLabel());
            this->lblVersion = (gcnew System::Windows::Forms::Label());
            this->DotNetLibVer = (gcnew System::Windows::Forms::Label());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureLogo))->BeginInit();
            this->SuspendLayout();
            // 
            // pictureLogo
            // 
            this->pictureLogo->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureLogo.Image")));
            this->pictureLogo->Location = System::Drawing::Point(1, 0);
            this->pictureLogo->Margin = System::Windows::Forms::Padding(2);
            this->pictureLogo->Name = L"pictureLogo";
            this->pictureLogo->Size = System::Drawing::Size(200, 200);
            this->pictureLogo->SizeMode = System::Windows::Forms::PictureBoxSizeMode::AutoSize;
            this->pictureLogo->TabIndex = 0;
            this->pictureLogo->TabStop = false;
            // 
            // buttonClose
            // 
            this->buttonClose->BackColor = System::Drawing::Color::LightGray;
            this->buttonClose->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->buttonClose->Location = System::Drawing::Point(239, 203);
            this->buttonClose->Margin = System::Windows::Forms::Padding(2);
            this->buttonClose->Name = L"buttonClose";
            this->buttonClose->Size = System::Drawing::Size(76, 28);
            this->buttonClose->TabIndex = 3;
            this->buttonClose->Text = L"Close";
            this->buttonClose->UseVisualStyleBackColor = false;
            this->buttonClose->Click += gcnew System::EventHandler(this, &AboutDisplayTmats::buttonClose_Click);
            // 
            // linkIrig106
            // 
            this->linkIrig106->AutoSize = true;
            this->linkIrig106->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->linkIrig106->Location = System::Drawing::Point(31, 209);
            this->linkIrig106->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->linkIrig106->Name = L"linkIrig106";
            this->linkIrig106->Size = System::Drawing::Size(136, 16);
            this->linkIrig106->TabIndex = 4;
            this->linkIrig106->TabStop = true;
            this->linkIrig106->Text = L"http://www.irig106.org/";
            this->linkIrig106->Click += gcnew System::EventHandler(this, &AboutDisplayTmats::linkIrig106_Click);
            // 
            // lblVersion
            // 
            this->lblVersion->AutoSize = true;
            this->lblVersion->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->lblVersion->Location = System::Drawing::Point(217, 44);
            this->lblVersion->Name = L"lblVersion";
            this->lblVersion->Size = System::Drawing::Size(117, 64);
            this->lblVersion->TabIndex = 5;
            this->lblVersion->Text = L"Display TMATS\r\nVerson x.y.z\r\nCopyright 20xx\r\nirig106.org";
            this->lblVersion->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // DotNetLibVer
            // 
            this->DotNetLibVer->AutoSize = true;
            this->DotNetLibVer->Enabled = false;
            this->DotNetLibVer->Location = System::Drawing::Point(217, 139);
            this->DotNetLibVer->Name = L"DotNetLibVer";
            this->DotNetLibVer->Size = System::Drawing::Size(95, 39);
            this->DotNetLibVer->TabIndex = 6;
            this->DotNetLibVer->Text = L"IRIG 106 .NET Lib\r\n  Version x.x.x\r\n  Date Time";
            this->DotNetLibVer->Visible = false;
            // 
            // AboutDisplayTmats
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackColor = System::Drawing::Color::White;
            this->CancelButton = this->buttonClose;
            this->ClientSize = System::Drawing::Size(346, 239);
            this->Controls->Add(this->DotNetLibVer);
            this->Controls->Add(this->lblVersion);
            this->Controls->Add(this->linkIrig106);
            this->Controls->Add(this->buttonClose);
            this->Controls->Add(this->pictureLogo);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->Margin = System::Windows::Forms::Padding(2);
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"AboutDisplayTmats";
            this->Text = L"About Display TMATS";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureLogo))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion


private: System::Void buttonClose_Click(System::Object^  sender, System::EventArgs^  e) 
    {
    this->Close();
    }

private: System::Void linkIrig106_Click(System::Object^  sender, System::EventArgs^  e) 
    {
    System::Diagnostics::Process::Start("http://www.irig106.org/");
    }

};
}
