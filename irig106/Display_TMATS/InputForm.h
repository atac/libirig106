/****************************************************************************

 InputForm.h - A .NET 2005 class that implements the TMATS reader input dialog

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

#include <time.h>
#include <sys\stat.h>
#include <sys\timeb.h>

#include "config.h"
#include "stdint.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"
#include "irig106cl.h"

using namespace System::Runtime::InteropServices;
using namespace System::Windows::Forms;


namespace I106Input {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

    using namespace Irig106;

	public ref class InputForm : public System::Windows::Forms::Form
	{

    // ----------------------------------------------------------------
    // Constructor / Destructor
    // ----------------------------------------------------------------

	public:
		InputForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here

            this->IrigIn       = new Irig106::Irig106Lib();
//          this->psuTmatsInfo = new SuTmatsInfo;
//          memset(this->psuTmatsInfo, 0, sizeof(SuTmatsInfo));

		}

    // ----------------------------------------------------------------

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~InputForm()
		{
			if (components)
			{
				delete components;
			}

            IrigIn->Close();

            delete this->IrigIn;
            //delete this->psuTmatsInfo;
            //this->psuTmatsInfo = NULL;

		}

    // ----------------------------------------------------------------
    // Class data
    // ----------------------------------------------------------------

    protected: 

//        SuTmatsInfo                * psuTmatsInfo;   // Decoded TMATS info tree

    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblAbout;
    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblDummy;
    protected: 






    protected: 
        Irig106::Irig106Lib        * IrigIn;


    // ----------------------------------------------------------------
    // Class methods
    // ----------------------------------------------------------------

    protected:
    System::Void ProcessIrigFile();
    System::Void DecodeDisplayTMATS();
    System::Void InputForm::DisplayChannels();
    System::Void InputForm::DisplayTree();
    System::Void InputForm::DisplayRaw();

    // ----------------------------------------------------------------
    // Form components
    // ----------------------------------------------------------------
    private: System::Windows::Forms::StatusStrip^  statusStrip1;
    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblStartTime;
    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblStopTime;

    private: System::Windows::Forms::Button^        cmdBrowse;
    private: System::Windows::Forms::TextBox^       txtFilename;
    private: System::Windows::Forms::TabControl^    tabTMATS;
    private: System::Windows::Forms::TabPage^       tabRaw;
    private: System::Windows::Forms::TextBox^       textRaw;
    private: System::Windows::Forms::TabPage^       tabChannels;
    private: System::Windows::Forms::TabPage^       tabTree;
    private: System::Windows::Forms::TextBox^       textChannels;
    private: System::Windows::Forms::TreeView^      treeTree;

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(InputForm::typeid));
            this->cmdBrowse = (gcnew System::Windows::Forms::Button());
            this->txtFilename = (gcnew System::Windows::Forms::TextBox());
            this->tabTMATS = (gcnew System::Windows::Forms::TabControl());
            this->tabRaw = (gcnew System::Windows::Forms::TabPage());
            this->textRaw = (gcnew System::Windows::Forms::TextBox());
            this->tabChannels = (gcnew System::Windows::Forms::TabPage());
            this->textChannels = (gcnew System::Windows::Forms::TextBox());
            this->tabTree = (gcnew System::Windows::Forms::TabPage());
            this->treeTree = (gcnew System::Windows::Forms::TreeView());
            this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
            this->statuslblStartTime = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statuslblStopTime = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statuslblAbout = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statuslblDummy = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->tabTMATS->SuspendLayout();
            this->tabRaw->SuspendLayout();
            this->tabChannels->SuspendLayout();
            this->tabTree->SuspendLayout();
            this->statusStrip1->SuspendLayout();
            this->SuspendLayout();
            // 
            // cmdBrowse
            // 
            this->cmdBrowse->Location = System::Drawing::Point(9, 10);
            this->cmdBrowse->Margin = System::Windows::Forms::Padding(2);
            this->cmdBrowse->Name = L"cmdBrowse";
            this->cmdBrowse->Size = System::Drawing::Size(55, 29);
            this->cmdBrowse->TabIndex = 0;
            this->cmdBrowse->Text = L"File";
            this->cmdBrowse->UseVisualStyleBackColor = true;
            this->cmdBrowse->Click += gcnew System::EventHandler(this, &InputForm::cmdBrowse_Click);
            // 
            // txtFilename
            // 
            this->txtFilename->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->txtFilename->Location = System::Drawing::Point(71, 15);
            this->txtFilename->Margin = System::Windows::Forms::Padding(2);
            this->txtFilename->Name = L"txtFilename";
            this->txtFilename->Size = System::Drawing::Size(472, 20);
            this->txtFilename->TabIndex = 1;
            this->txtFilename->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &InputForm::txtFilename_KeyPress);
            // 
            // tabTMATS
            // 
            this->tabTMATS->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->tabTMATS->Controls->Add(this->tabRaw);
            this->tabTMATS->Controls->Add(this->tabChannels);
            this->tabTMATS->Controls->Add(this->tabTree);
            this->tabTMATS->Location = System::Drawing::Point(3, 45);
            this->tabTMATS->Margin = System::Windows::Forms::Padding(2);
            this->tabTMATS->Name = L"tabTMATS";
            this->tabTMATS->SelectedIndex = 0;
            this->tabTMATS->Size = System::Drawing::Size(540, 441);
            this->tabTMATS->TabIndex = 2;
            // 
            // tabRaw
            // 
            this->tabRaw->Controls->Add(this->textRaw);
            this->tabRaw->Location = System::Drawing::Point(4, 22);
            this->tabRaw->Margin = System::Windows::Forms::Padding(2);
            this->tabRaw->Name = L"tabRaw";
            this->tabRaw->Padding = System::Windows::Forms::Padding(2);
            this->tabRaw->Size = System::Drawing::Size(532, 415);
            this->tabRaw->TabIndex = 0;
            this->tabRaw->Text = L"Raw";
            this->tabRaw->UseVisualStyleBackColor = true;
            // 
            // textRaw
            // 
            this->textRaw->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->textRaw->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->textRaw->Location = System::Drawing::Point(0, 2);
            this->textRaw->Margin = System::Windows::Forms::Padding(2);
            this->textRaw->Multiline = true;
            this->textRaw->Name = L"textRaw";
            this->textRaw->ScrollBars = System::Windows::Forms::ScrollBars::Both;
            this->textRaw->Size = System::Drawing::Size(532, 413);
            this->textRaw->TabIndex = 0;
            // 
            // tabChannels
            // 
            this->tabChannels->Controls->Add(this->textChannels);
            this->tabChannels->Location = System::Drawing::Point(4, 22);
            this->tabChannels->Margin = System::Windows::Forms::Padding(2);
            this->tabChannels->Name = L"tabChannels";
            this->tabChannels->Padding = System::Windows::Forms::Padding(2);
            this->tabChannels->Size = System::Drawing::Size(504, 290);
            this->tabChannels->TabIndex = 1;
            this->tabChannels->Text = L"Channels";
            this->tabChannels->UseVisualStyleBackColor = true;
            // 
            // textChannels
            // 
            this->textChannels->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->textChannels->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->textChannels->Location = System::Drawing::Point(0, 2);
            this->textChannels->Margin = System::Windows::Forms::Padding(2);
            this->textChannels->Multiline = true;
            this->textChannels->Name = L"textChannels";
            this->textChannels->ScrollBars = System::Windows::Forms::ScrollBars::Both;
            this->textChannels->Size = System::Drawing::Size(504, 296);
            this->textChannels->TabIndex = 0;
            // 
            // tabTree
            // 
            this->tabTree->Controls->Add(this->treeTree);
            this->tabTree->Location = System::Drawing::Point(4, 22);
            this->tabTree->Margin = System::Windows::Forms::Padding(2);
            this->tabTree->Name = L"tabTree";
            this->tabTree->Padding = System::Windows::Forms::Padding(2);
            this->tabTree->Size = System::Drawing::Size(504, 290);
            this->tabTree->TabIndex = 2;
            this->tabTree->Text = L"Tree";
            this->tabTree->UseVisualStyleBackColor = true;
            // 
            // treeTree
            // 
            this->treeTree->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->treeTree->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->treeTree->Location = System::Drawing::Point(0, 2);
            this->treeTree->Margin = System::Windows::Forms::Padding(2);
            this->treeTree->Name = L"treeTree";
            this->treeTree->Size = System::Drawing::Size(504, 296);
            this->treeTree->TabIndex = 0;
            // 
            // statusStrip1
            // 
            this->statusStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Visible;
            this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->statuslblStartTime, 
                this->statuslblStopTime, this->statuslblAbout, this->statuslblDummy});
            this->statusStrip1->Location = System::Drawing::Point(0, 488);
            this->statusStrip1->Name = L"statusStrip1";
            this->statusStrip1->Padding = System::Windows::Forms::Padding(1, 0, 10, 0);
            this->statusStrip1->Size = System::Drawing::Size(547, 25);
            this->statusStrip1->TabIndex = 4;
            this->statusStrip1->Text = L"statusFileInfo";
            // 
            // statuslblStartTime
            // 
            this->statuslblStartTime->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
            this->statuslblStartTime->BorderStyle = System::Windows::Forms::Border3DStyle::Sunken;
            this->statuslblStartTime->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->statuslblStartTime->Name = L"statuslblStartTime";
            this->statuslblStartTime->Size = System::Drawing::Size(228, 20);
            this->statuslblStartTime->Spring = true;
            this->statuslblStartTime->Text = L"Start";
            // 
            // statuslblStopTime
            // 
            this->statuslblStopTime->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
            this->statuslblStopTime->BorderStyle = System::Windows::Forms::Border3DStyle::Sunken;
            this->statuslblStopTime->Name = L"statuslblStopTime";
            this->statuslblStopTime->Size = System::Drawing::Size(228, 20);
            this->statuslblStopTime->Spring = true;
            this->statuslblStopTime->Text = L"Stop";
            // 
            // statuslblAbout
            // 
            this->statuslblAbout->AutoToolTip = true;
            this->statuslblAbout->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
                | System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
            this->statuslblAbout->BorderStyle = System::Windows::Forms::Border3DStyle::Raised;
            this->statuslblAbout->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"statuslblAbout.Image")));
            this->statuslblAbout->Name = L"statuslblAbout";
            this->statuslblAbout->Size = System::Drawing::Size(69, 20);
            this->statuslblAbout->Text = L"About...";
            this->statuslblAbout->ToolTipText = L"www.irig106.org";
            this->statuslblAbout->Click += gcnew System::EventHandler(this, &InputForm::statuslblAbout_Click);
            // 
            // statuslblDummy
            // 
            this->statuslblDummy->AutoSize = false;
            this->statuslblDummy->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::None;
            this->statuslblDummy->Name = L"statuslblDummy";
            this->statuslblDummy->Size = System::Drawing::Size(10, 20);
            this->statuslblDummy->Text = L" ";
            // 
            // InputForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(547, 513);
            this->Controls->Add(this->statusStrip1);
            this->Controls->Add(this->tabTMATS);
            this->Controls->Add(this->txtFilename);
            this->Controls->Add(this->cmdBrowse);
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->Margin = System::Windows::Forms::Padding(2);
            this->Name = L"InputForm";
            this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
            this->Text = L"Display TMATS - irig106.org";
            this->tabTMATS->ResumeLayout(false);
            this->tabRaw->ResumeLayout(false);
            this->tabRaw->PerformLayout();
            this->tabChannels->ResumeLayout(false);
            this->tabChannels->PerformLayout();
            this->tabTree->ResumeLayout(false);
            this->statusStrip1->ResumeLayout(false);
            this->statusStrip1->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion


    // ----------------------------------------------------------------
    // Events
    // ----------------------------------------------------------------

private: System::Void cmdBrowse_Click(System::Object^  sender, System::EventArgs^  e)
    {
    ::DialogResult      DStatus;

    // Open the file dialog        
    OpenFileDialog ^ dlgOpenFile = gcnew OpenFileDialog();
    dlgOpenFile->DefaultExt = "c10";
    dlgOpenFile->Filter     = "Ch 10 files (*.c10,*.ch10)|*.c10;*.ch10|All files (*.*)|*.*";
    DStatus = dlgOpenFile->ShowDialog();

    // If file selected then process the file
    if (DStatus == ::DialogResult::OK)
        {
        // Put the filename in the form for all to see        
        txtFilename->Text = dlgOpenFile->FileName;

        // Now process the data file
        ProcessIrigFile();
        }

    } // end cmdBrowse_Click()



// --------------------------------------------------------------------------

// Look for user pressing ENTER in filename text box

private: System::Void txtFilename_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) 
    {
    if (e->KeyChar == (char)13)
        {
        e->Handled = true;
        ProcessIrigFile();
        } // end if ENTER
    }

// --------------------------------------------------------------------------

// Put up the "About" dialog

private: System::Void statuslblAbout_Click(System::Object^  sender, System::EventArgs^  e) 
    {
    AboutDisplayTmats ^ About = gcnew AboutDisplayTmats();
    About->Show();
    }

}; // end of Input_Form class

} // end of I106Input namespace

