#pragma once

//#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <sys\stat.h>
#include <sys\timeb.h>

#include <list>

#include "config.h"
#include "stdint.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"
#include "irig106cl.h"

using namespace std;

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

using namespace Irig106;


namespace i106Dub {

    struct FilePoint
        {
        int64_t             llOffset;  // File offset
        int64_t             llRtcTime; // Relative time
        SuIrig106Time       suITime;   // IRIG time
        };

	public ref class SelectForm : public System::Windows::Forms::Form
	{

// ------------------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------------------

	public:
		SelectForm(void)
    		{
			InitializeComponent();

            IrigIn = new Irig106::Irig106Lib();

            Channel0Info = gcnew SuChannel0Info;
            ChannelInfo  = gcnew List<SuChannelInfo^>();

            psuDubBegin      = new FilePoint;
            psuDubEnd        = new FilePoint;

            psuFileBeginTime = new SuIrig106Time;
            psuFileEndTime   = new SuIrig106Time;

	    	}

// ------------------------------------------------------------------------

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SelectForm()
		    {
			if (components)
			{
				delete components;
			}

            IrigIn->Close();
            delete IrigIn;

            delete psuDubBegin;
            delete psuDubEnd;

            delete this->psuFileBeginTime;
            delete this->psuFileEndTime;
		    }


// ------------------------------------------------------------------------
// Class functions, variables, and controls
// ------------------------------------------------------------------------

    // Declarations
    ref struct SuChannel0Info
        {
        CheckBox      ^ pCopyTMATSCheckBox;
        CheckBox      ^ pIndexTMATSCheckBox;
        CheckBox      ^ pCopyEventsCheckBox;
        CheckBox      ^ pIndexEventsCheckBox;
        CheckBox      ^ pCopyOtherCheckBox;
        CheckBox      ^ pIndexOtherCheckBox;
        };

    ref struct SuChannelInfo
        {
        CheckBox      ^ pCopyCheckBox;
        CheckBox      ^ pIndexCheckBox;
        int             iChannelNum;
        String        ^ ChannelType;
        };

    // Functions
    protected:
    System::Void InitFormFromIrig();
    System::Void PopulateChannelTable();
    System::Void SelectForm::SetTableHeight();

    // Class variables
    protected: 
        Irig106Lib                    * IrigIn;

        SuIrig106Time                 * psuFileBeginTime;   // File begin time
        SuIrig106Time                 * psuFileEndTime;     // File end time
        long long                       llFileSize;
        FilePoint                     * psuDubBegin;
        FilePoint                     * psuDubEnd;

        SuChannel0Info                ^ Channel0Info;
        List<SuChannelInfo ^>         ^ ChannelInfo;

    // Form controls
    private: System::Windows::Forms::TableLayoutPanel^      tlpChannels;
    private: System::Windows::Forms::TrackBar^              traStartTime;
    private: System::Windows::Forms::TrackBar^              traStopTime;
    private: System::Windows::Forms::StatusStrip^           statusStrip1;
    private: System::Windows::Forms::Button^                cmdInput;
    private: System::Windows::Forms::Button^                cmdOutput;
    private: System::Windows::Forms::TextBox^               txtStartTime;
    private: System::Windows::Forms::TextBox^               txtStopTime;
    private: System::Windows::Forms::TextBox^               txtInFile;
    private: System::Windows::Forms::TextBox^               txtOutFile;
    private: System::Windows::Forms::Button^                cmdDub;
    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblStartTime;
    private: System::Windows::Forms::ToolStripStatusLabel^  statuslblStopTime;
    private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel3;
    private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabelDummy;
    private: System::Windows::Forms::Label^                 label1;
    private: System::Windows::Forms::Label^                 label2;

	private: System::ComponentModel::Container^             components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(SelectForm::typeid));
			this->tlpChannels = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->traStartTime = (gcnew System::Windows::Forms::TrackBar());
			this->traStopTime = (gcnew System::Windows::Forms::TrackBar());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->statuslblStartTime = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->statuslblStopTime = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabel3 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabelDummy = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->cmdInput = (gcnew System::Windows::Forms::Button());
			this->cmdOutput = (gcnew System::Windows::Forms::Button());
			this->txtStartTime = (gcnew System::Windows::Forms::TextBox());
			this->txtStopTime = (gcnew System::Windows::Forms::TextBox());
			this->txtInFile = (gcnew System::Windows::Forms::TextBox());
			this->txtOutFile = (gcnew System::Windows::Forms::TextBox());
			this->cmdDub = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->traStartTime))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->traStopTime))->BeginInit();
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// tlpChannels
			// 
			this->tlpChannels->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->tlpChannels->AutoScroll = true;
			this->tlpChannels->BackColor = System::Drawing::SystemColors::Control;
			this->tlpChannels->CellBorderStyle = System::Windows::Forms::TableLayoutPanelCellBorderStyle::InsetDouble;
			this->tlpChannels->ColumnCount = 4;
			this->tlpChannels->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tlpChannels->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tlpChannels->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tlpChannels->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 40)));
			this->tlpChannels->Location = System::Drawing::Point(9, 62);
			this->tlpChannels->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->tlpChannels->Name = L"tlpChannels";
			this->tlpChannels->Padding = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->tlpChannels->RowCount = 2;
			this->tlpChannels->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, 16)));
			this->tlpChannels->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tlpChannels->Size = System::Drawing::Size(477, 434);
			this->tlpChannels->TabIndex = 0;
			// 
			// traStartTime
			// 
			this->traStartTime->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->traStartTime->AutoSize = false;
			this->traStartTime->BackColor = System::Drawing::SystemColors::Control;
			this->traStartTime->Enabled = false;
			this->traStartTime->Location = System::Drawing::Point(181, 500);
			this->traStartTime->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->traStartTime->Maximum = 1000;
			this->traStartTime->Name = L"traStartTime";
			this->traStartTime->Size = System::Drawing::Size(306, 24);
			this->traStartTime->TabIndex = 2;
			this->traStartTime->TickFrequency = 50;
			this->traStartTime->TickStyle = System::Windows::Forms::TickStyle::None;
			this->traStartTime->ValueChanged += gcnew System::EventHandler(this, &SelectForm::traStartTime_ValueChanged);
			this->traStartTime->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &SelectForm::traStartTime_MouseUp);
			// 
			// traStopTime
			// 
			this->traStopTime->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->traStopTime->AutoSize = false;
			this->traStopTime->BackColor = System::Drawing::SystemColors::Control;
			this->traStopTime->Enabled = false;
			this->traStopTime->Location = System::Drawing::Point(181, 529);
			this->traStopTime->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->traStopTime->Maximum = 1000;
			this->traStopTime->Name = L"traStopTime";
			this->traStopTime->Size = System::Drawing::Size(306, 24);
			this->traStopTime->TabIndex = 3;
			this->traStopTime->TickFrequency = 50;
			this->traStopTime->TickStyle = System::Windows::Forms::TickStyle::None;
			this->traStopTime->Value = 1000;
			this->traStopTime->ValueChanged += gcnew System::EventHandler(this, &SelectForm::traStopTime_ValueChanged);
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->statuslblStartTime, 
				this->statuslblStopTime, this->toolStripStatusLabel3, this->toolStripStatusLabelDummy});
			this->statusStrip1->Location = System::Drawing::Point(0, 560);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Padding = System::Windows::Forms::Padding(1, 0, 10, 0);
			this->statusStrip1->Size = System::Drawing::Size(497, 27);
			this->statusStrip1->TabIndex = 4;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// statuslblStartTime
			// 
			this->statuslblStartTime->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
			this->statuslblStartTime->BorderStyle = System::Windows::Forms::Border3DStyle::Sunken;
			this->statuslblStartTime->Name = L"statuslblStartTime";
			this->statuslblStartTime->Size = System::Drawing::Size(197, 22);
			this->statuslblStartTime->Spring = true;
			this->statuslblStartTime->Text = L"Begin -";
			// 
			// statuslblStopTime
			// 
			this->statuslblStopTime->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
			this->statuslblStopTime->BorderStyle = System::Windows::Forms::Border3DStyle::Sunken;
			this->statuslblStopTime->Name = L"statuslblStopTime";
			this->statuslblStopTime->Size = System::Drawing::Size(197, 22);
			this->statuslblStopTime->Spring = true;
			this->statuslblStopTime->Text = L"End -";
			// 
			// toolStripStatusLabel3
			// 
			this->toolStripStatusLabel3->AutoSize = false;
			this->toolStripStatusLabel3->BorderSides = static_cast<System::Windows::Forms::ToolStripStatusLabelBorderSides>((((System::Windows::Forms::ToolStripStatusLabelBorderSides::Left | System::Windows::Forms::ToolStripStatusLabelBorderSides::Top) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Right) 
				| System::Windows::Forms::ToolStripStatusLabelBorderSides::Bottom));
			this->toolStripStatusLabel3->BorderStyle = System::Windows::Forms::Border3DStyle::Raised;
			this->toolStripStatusLabel3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripStatusLabel3.Image")));
			this->toolStripStatusLabel3->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->toolStripStatusLabel3->Name = L"toolStripStatusLabel3";
			this->toolStripStatusLabel3->Size = System::Drawing::Size(81, 22);
			this->toolStripStatusLabel3->Text = L"About...";
			this->toolStripStatusLabel3->Click += gcnew System::EventHandler(this, &SelectForm::toolStripStatusLabel3_Click);
			// 
			// toolStripStatusLabelDummy
			// 
			this->toolStripStatusLabelDummy->AutoSize = false;
			this->toolStripStatusLabelDummy->Name = L"toolStripStatusLabelDummy";
			this->toolStripStatusLabelDummy->Size = System::Drawing::Size(10, 22);
			// 
			// cmdInput
			// 
			this->cmdInput->Location = System::Drawing::Point(9, 7);
			this->cmdInput->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->cmdInput->Name = L"cmdInput";
			this->cmdInput->Size = System::Drawing::Size(49, 23);
			this->cmdInput->TabIndex = 5;
			this->cmdInput->Text = L"In File";
			this->cmdInput->UseVisualStyleBackColor = true;
			this->cmdInput->Click += gcnew System::EventHandler(this, &SelectForm::cmdInput_Click);
			// 
			// cmdOutput
			// 
			this->cmdOutput->Location = System::Drawing::Point(9, 35);
			this->cmdOutput->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->cmdOutput->Name = L"cmdOutput";
			this->cmdOutput->Size = System::Drawing::Size(49, 23);
			this->cmdOutput->TabIndex = 6;
			this->cmdOutput->Text = L"Out File";
			this->cmdOutput->UseVisualStyleBackColor = true;
			this->cmdOutput->Click += gcnew System::EventHandler(this, &SelectForm::cmdOutput_Click);
			// 
			// txtStartTime
			// 
			this->txtStartTime->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->txtStartTime->Location = System::Drawing::Point(66, 506);
			this->txtStartTime->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->txtStartTime->Name = L"txtStartTime";
			this->txtStartTime->Size = System::Drawing::Size(111, 20);
			this->txtStartTime->TabIndex = 7;
			// 
			// txtStopTime
			// 
			this->txtStopTime->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->txtStopTime->Location = System::Drawing::Point(66, 538);
			this->txtStopTime->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->txtStopTime->Name = L"txtStopTime";
			this->txtStopTime->Size = System::Drawing::Size(111, 20);
			this->txtStopTime->TabIndex = 8;
			// 
			// txtInFile
			// 
			this->txtInFile->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->txtInFile->Location = System::Drawing::Point(62, 10);
			this->txtInFile->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->txtInFile->Name = L"txtInFile";
			this->txtInFile->Size = System::Drawing::Size(348, 20);
			this->txtInFile->TabIndex = 9;
			// 
			// txtOutFile
			// 
			this->txtOutFile->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->txtOutFile->Location = System::Drawing::Point(62, 37);
			this->txtOutFile->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->txtOutFile->Name = L"txtOutFile";
			this->txtOutFile->Size = System::Drawing::Size(348, 20);
			this->txtOutFile->TabIndex = 10;
			// 
			// cmdDub
			// 
			this->cmdDub->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->cmdDub->Enabled = false;
			this->cmdDub->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->cmdDub->Location = System::Drawing::Point(414, 10);
			this->cmdDub->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->cmdDub->Name = L"cmdDub";
			this->cmdDub->Size = System::Drawing::Size(74, 46);
			this->cmdDub->TabIndex = 11;
			this->cmdDub->Text = L"Start Dub";
			this->cmdDub->UseVisualStyleBackColor = true;
			this->cmdDub->Click += gcnew System::EventHandler(this, &SelectForm::cmdDub_Click);
			// 
			// label1
			// 
			this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(8, 509);
			this->label1->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(55, 13);
			this->label1->TabIndex = 12;
			this->label1->Text = L"Start Time";
			// 
			// label2
			// 
			this->label2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(8, 540);
			this->label2->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(55, 13);
			this->label2->TabIndex = 13;
			this->label2->Text = L"Stop Time";
			// 
			// SelectForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(497, 587);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->cmdDub);
			this->Controls->Add(this->txtOutFile);
			this->Controls->Add(this->txtInFile);
			this->Controls->Add(this->txtStopTime);
			this->Controls->Add(this->txtStartTime);
			this->Controls->Add(this->cmdOutput);
			this->Controls->Add(this->cmdInput);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->traStopTime);
			this->Controls->Add(this->traStartTime);
			this->Controls->Add(this->tlpChannels);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->Name = L"SelectForm";
			this->Text = L"IRIG 106 Dub - irig106.org";
			this->SizeChanged += gcnew System::EventHandler(this, &SelectForm::SelectForm_SizeChanged);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->traStartTime))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->traStopTime))->EndInit();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

// ------------------------------------------------------------------------
// Events
// ------------------------------------------------------------------------

    private: System::Void toolStripStatusLabel3_Click(System::Object^  sender, System::EventArgs^  e) 
        {
        AboutDub ^ About = gcnew AboutDub();
        About->Show();
        }



    // --------------------------------------------------------------------
    // Choose input file name
    private: System::Void cmdInput_Click(System::Object^  sender, System::EventArgs^  e) 
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
            txtInFile->Text = dlgOpenFile->FileName;

            // Fill in form with data file info
            InitFormFromIrig();
            } // end if open OK

        // If we've got an input and output file name then enable the dub button
        if ((this->txtInFile->Text->Length  != 0) &&
            (this->txtOutFile->Text->Length != 0))
            this->cmdDub->Enabled = true;

        }



    // --------------------------------------------------------------------
    // Choose output file name
    private: System::Void cmdOutput_Click(System::Object^  sender, System::EventArgs^  e) 
        {
        ::DialogResult      DStatus;

        // Open the file dialog        
        OpenFileDialog ^ dlgOpenFile = gcnew OpenFileDialog();
        dlgOpenFile->DefaultExt = "c10";
        dlgOpenFile->Filter     = "Ch 10 files (*.c10,*.ch10)|*.c10;*.ch10|All files (*.*)|*.*";
        dlgOpenFile->CheckFileExists = false;
        DStatus = dlgOpenFile->ShowDialog();

        // If file selected then process the file
        if (DStatus == ::DialogResult::OK)
            {
            // Put the filename in the form for all to see        
            txtOutFile->Text = dlgOpenFile->FileName;
            } // end if open OK

        // If we've got an input and output file name then enable the dub button
        if ((this->txtInFile->Text->Length  != 0) &&
            (this->txtOutFile->Text->Length != 0))
            this->cmdDub->Enabled = true;

        }



    // --------------------------------------------------------------------
    // Begin the dub process
    private: System::Void cmdDub_Click(System::Object^  sender, System::EventArgs^  e);



    // --------------------------------------------------------------------
    // Start time slider changed

    private: System::Void traStartTime_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
        {
        int             iSliderVal;
        int             iSliderMax;
        int64_t         iFilePos;

        // Set the relative file position
        iSliderVal = traStartTime->Value;
        iSliderMax = traStartTime->Maximum;
        iFilePos = int64_t(double(iSliderVal) / double(iSliderMax) * double(llFileSize));
        IrigIn->SetPos(iFilePos);

        // Get some info about where we're currently at in the file
        IrigIn->GetPos(psuDubBegin->llOffset);
        IrigIn->ReadNextHeader();

        // Don't start at TMATS, it messes up the real data start time
        if (IrigIn->pHeader->ubyDataType == I106CH10_DTYPE_TMATS)
            IrigIn->ReadNextHeader();
        IrigIn->Rel2IrigTime(&(psuDubBegin->suITime));
        IrigIn->TimeArray2LLInt(IrigIn->pHeader->aubyRefTime, &(psuDubBegin->llRtcTime));

        // Update the time display
        txtStartTime->Text = IrigIn->strTime2String(&(psuDubBegin->suITime));

        // Make sure stop time slider is >= start time slider
        if (traStopTime->Value < traStartTime->Value)
            traStopTime->Value = traStartTime->Value;
        }



    // --------------------------------------------------------------------
    // Stop time slider changed
    private: System::Void traStopTime_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
        {
        int             iSliderVal;
        int             iSliderMax;

        // Set the relative file position
        iSliderVal = traStopTime->Value;
        iSliderMax = traStopTime->Maximum;
        IrigIn->SetPos(int64_t(double(iSliderVal) / double(iSliderMax) * double(llFileSize)));

        // Get some info about where we're currently at in the file
        IrigIn->GetPos(psuDubEnd->llOffset);
        IrigIn->ReadNextHeader();
        IrigIn->Rel2IrigTime(&(psuDubEnd->suITime));
        IrigIn->TimeArray2LLInt(IrigIn->pHeader->aubyRefTime, &(psuDubEnd->llRtcTime));

        // Update the time display
        txtStopTime->Text = IrigIn->strTime2String(&(psuDubEnd->suITime));

        // Make sure start time slider is <= stop time slider
        if (traStartTime->Value > traStopTime->Value)
            traStartTime->Value = traStopTime->Value;
        }

    // --------------------------------------------------------------------
    // Done with mouse so resync time
    private: System::Void traStartTime_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) 
        {

        // Get some info about where we're currently at in the file
        IrigIn->GetPos(psuDubBegin->llOffset);
        IrigIn->ReadNextHeader();
        IrigIn->SetPos(psuDubBegin->llOffset);

        IrigIn->SyncTime();
        IrigIn->Rel2IrigTime(&(psuDubBegin->suITime));
        IrigIn->TimeArray2LLInt(IrigIn->pHeader->aubyRefTime, &(psuDubBegin->llRtcTime));

        // Update the time display
        txtStartTime->Text = IrigIn->strTime2String(&(psuDubBegin->suITime));
        }


    // --------------------------------------------------------------------
    // Form size changed so update that goofy TableLayoutPanel
    private: System::Void SelectForm_SizeChanged(System::Object^  sender, System::EventArgs^  e) 
        {
        SetTableHeight();
        }

    }; // end class SelectForm

}
