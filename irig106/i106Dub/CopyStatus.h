/****************************************************************************

 CopyStatus.h - Dialog to display copy progress.

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

//#include "config.h"
//#include "stdint.h"
//#include "irig106ch10.h"
//#include "i106_time.h"
//#include "i106_decode_index.h"
//#include "i106_decode_tmats.h"
//#include "irig106cl.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace i106Dub {

public ref class CopyStatus : public System::Windows::Forms::Form
	{

// ------------------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------------------

	public:
		CopyStatus()
	    	{
            // Initialize the form components
			InitializeComponent();

            // Setup the user buttons
            cmdCancel->Enabled = false;
            cmdClose->Enabled  = true;

            // Setup the background thread stuff
            CopyThread->DoWork             += gcnew DoWorkEventHandler            (this, &CopyStatus::CopyDo);
            CopyThread->ProgressChanged    += gcnew ProgressChangedEventHandler   (this, &CopyStatus::CopyUpdate);
            CopyThread->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &CopyStatus::CopyDone);

            // Initialize sequence numbers
////            ChanSeqNums = gcnew array<uint8_t>(0x10000);
            Array::Clear(ChanSeqNums,0,ChanSeqNums->Length);

    		}

// ------------------------------------------------------------------------
	protected:
		~CopyStatus()
    		{
			if (components)
	    		{
				delete components;
		    	}
		    }

// ------------------------------------------------------------------------
// Class functions, variables, and controls
// ------------------------------------------------------------------------

    // Declarations
    public:

    ref struct SuCopyParams
        {
        // Elements
        String        ^ sInFile;
        String        ^ sOutFile;
        __int64         llBeginOffset;
        __int64         llEndOffset;
        bool            bSkipTime;
        bool            bCh0TMATSCopy;
        bool            bCh0TMATSIndex;
        bool            bCh0EventsCopy;
        bool            bCh0EventsIndex;
        bool            bCh0OtherCopy;
        bool            bCh0OtherIndex;
        array<char>   ^ ChanEnabled;
        array<char>   ^ ChanIndex;

        // Constructor
        SuCopyParams()
            {
            ChanEnabled = gcnew array<char>(0x10000);
            ChanIndex   = gcnew array<char>(0x10000);
            }
        }; // end of SuCopyParams

    ref struct SuPacketIndex
        {
        __int64                     llTime; 
        unsigned __int16            uChannelID;
        Irig106DotNet::DataType     uDataType;
        __int64                     uOffset;
        };

    // Functions
    System::Void CopyStatus::DubFile(BackgroundWorker^ bCaller, DoWorkEventArgs^ mArgs);

    // Class variables
    protected: 

        // List of current per channel sequence number
        array<__int8>    ^ ChanSeqNums;

    // Form controls
    private: System::Windows::Forms::ProgressBar^       pgbCopyStatus;
    private: System::Windows::Forms::PictureBox^        picLogo;
    private: System::Windows::Forms::Button^            cmdCancel;
    private: System::Windows::Forms::TextBox^           txtStatus;
    private: System::ComponentModel::BackgroundWorker^  CopyThread;
    private: System::Windows::Forms::Button^            cmdClose;

	private: System::ComponentModel::Container^         components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(CopyStatus::typeid));
            this->pgbCopyStatus = (gcnew System::Windows::Forms::ProgressBar());
            this->picLogo = (gcnew System::Windows::Forms::PictureBox());
            this->cmdCancel = (gcnew System::Windows::Forms::Button());
            this->txtStatus = (gcnew System::Windows::Forms::TextBox());
            this->CopyThread = (gcnew System::ComponentModel::BackgroundWorker());
            this->cmdClose = (gcnew System::Windows::Forms::Button());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->picLogo))->BeginInit();
            this->SuspendLayout();
            // 
            // pgbCopyStatus
            // 
            this->pgbCopyStatus->ForeColor = System::Drawing::Color::DarkOrange;
            this->pgbCopyStatus->Location = System::Drawing::Point(139, 87);
            this->pgbCopyStatus->Margin = System::Windows::Forms::Padding(4);
            this->pgbCopyStatus->Name = L"pgbCopyStatus";
            this->pgbCopyStatus->Size = System::Drawing::Size(319, 30);
            this->pgbCopyStatus->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
            this->pgbCopyStatus->TabIndex = 0;
            // 
            // picLogo
            // 
            this->picLogo->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"picLogo.Image")));
            this->picLogo->Location = System::Drawing::Point(16, 15);
            this->picLogo->Margin = System::Windows::Forms::Padding(4);
            this->picLogo->Name = L"picLogo";
            this->picLogo->Size = System::Drawing::Size(109, 101);
            this->picLogo->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->picLogo->TabIndex = 2;
            this->picLogo->TabStop = false;
            // 
            // cmdCancel
            // 
            this->cmdCancel->Enabled = false;
            this->cmdCancel->Location = System::Drawing::Point(472, 13);
            this->cmdCancel->Margin = System::Windows::Forms::Padding(4);
            this->cmdCancel->Name = L"cmdCancel";
            this->cmdCancel->Size = System::Drawing::Size(111, 42);
            this->cmdCancel->TabIndex = 4;
            this->cmdCancel->Text = L"Cancel";
            this->cmdCancel->UseVisualStyleBackColor = true;
            this->cmdCancel->Click += gcnew System::EventHandler(this, &CopyStatus::cmdCancel_Click);
            // 
            // txtStatus
            // 
            this->txtStatus->BackColor = System::Drawing::SystemColors::Control;
            this->txtStatus->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->txtStatus->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->txtStatus->Location = System::Drawing::Point(139, 15);
            this->txtStatus->Multiline = true;
            this->txtStatus->Name = L"txtStatus";
            this->txtStatus->Size = System::Drawing::Size(319, 55);
            this->txtStatus->TabIndex = 5;
            // 
            // CopyThread
            // 
            this->CopyThread->WorkerReportsProgress = true;
            this->CopyThread->WorkerSupportsCancellation = true;
            // 
            // cmdClose
            // 
            this->cmdClose->Location = System::Drawing::Point(472, 74);
            this->cmdClose->Margin = System::Windows::Forms::Padding(4);
            this->cmdClose->Name = L"cmdClose";
            this->cmdClose->Size = System::Drawing::Size(111, 42);
            this->cmdClose->TabIndex = 6;
            this->cmdClose->Text = L"Close";
            this->cmdClose->UseVisualStyleBackColor = true;
            this->cmdClose->Click += gcnew System::EventHandler(this, &CopyStatus::cmdClose_Click);
            // 
            // CopyStatus
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(596, 132);
            this->Controls->Add(this->cmdClose);
            this->Controls->Add(this->txtStatus);
            this->Controls->Add(this->cmdCancel);
            this->Controls->Add(this->picLogo);
            this->Controls->Add(this->pgbCopyStatus);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->Margin = System::Windows::Forms::Padding(4);
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"CopyStatus";
            this->Text = L"Copy Status";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->picLogo))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion


// ------------------------------------------------------------------------
// Public methods
// ------------------------------------------------------------------------
    public:
    System::Void StartCopy(SuCopyParams ^ mParamArgs)
        {
        SuCopyParams  ^ mParams = gcnew SuCopyParams;

        // Setup the user buttons
        cmdCancel->Enabled = true;
        cmdClose->Enabled  = false;

        // Get a copy of the parameters to pass to the background thread
        mParams = mParamArgs;

        // Now start the background copy thread
        CopyThread->RunWorkerAsync(mParams);

        }



// ------------------------------------------------------------------------
// Events
// ------------------------------------------------------------------------

    private: System::Void cmdCancel_Click(System::Object^  sender, System::EventArgs^  e)   
        {
        CopyThread->CancelAsync();
        }



// ------------------------------------------------------------------------

private: System::Void cmdClose_Click(System::Object^  sender, System::EventArgs^  e) 
    {
    this->Close();
    } // end close button event



// ------------------------------------------------------------------------
// Background copy thread stuff
// ------------------------------------------------------------------------

    // This is the delegate that will be the background thread.  It is 
    // a delegate of the form...
    // public delegate void DoWorkEventHandler (Object^ sender,	DoWorkEventArgs^ e)

    void CopyDo(Object^ oCaller, DoWorkEventArgs^ mArgs)
        {
        // Get the BackgroundWorker that raised this event.
        BackgroundWorker^ bCaller = dynamic_cast<BackgroundWorker^>(oCaller);

        DubFile(bCaller, mArgs);

        }



// ------------------------------------------------------------------------

    // This delegate is called when the thread wants to post a change to
    // it's current status.  It is a delegate of the form...
    // public delegate void ProgressChangedEventHandler(Object^ sender, ProgressChangedEventArgs^ e)

    void CopyUpdate(Object^ /*sender*/, ProgressChangedEventArgs^ mArgs)
        {
        this->txtStatus->Text = safe_cast<String ^>(mArgs->UserState);
        this->pgbCopyStatus->Value = mArgs->ProgressPercentage;
        }



// ------------------------------------------------------------------------

    // This is the delegate the is called when the thread is done. It is 
    // a delegate of the form...
    // public delegate void RunWorkerCompletedEventHandler(Object^ sender, RunWorkerCompletedEventArgs^ e)

    void CopyDone(Object^ /*sender*/, RunWorkerCompletedEventArgs^ mArgs)
        {
        // First, handle the case where an exception was thrown.
        if ( mArgs->Error != nullptr )
            {
            MessageBox::Show( mArgs->Error->Message );
            }

        // No exception thrown
        else
            // User canceled
            if ( mArgs->Cancelled )
                {
                txtStatus->Text = "Status : Cancelled!";
                }

            // Normal exit
            else
                {
                txtStatus->Text = "Status : Done!";
                }

        // Setup the user buttons
        cmdCancel->Enabled = false;
        cmdClose->Enabled  = true;

        } // end CopyDone



// ------------------------------------------------------------------------
// Utilities
// ------------------------------------------------------------------------

    public:
    System::Void UpdateStatus(String ^ sStatusLabel, float fCopyPercent)
        {
        this->txtStatus->Text = sStatusLabel;
        if (fCopyPercent > 1.0) fCopyPercent = 1.0;
        if (fCopyPercent < 0.0) fCopyPercent = 0.0;
        this->pgbCopyStatus->Value = int(this->pgbCopyStatus->Maximum * fCopyPercent);

        //this->Show();
        this->Refresh();
        this->Update();
        }

    protected:
    System::Void CopyStatus::FixSeqNum(Irig106DotNet::SuI106Ch10Header ^ IrigHeader);

}; // end class CopyStatus

} // end namespace i106Dub
