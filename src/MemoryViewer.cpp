#include "MemoryViewer.h"

#define X(opCode, name, val) name,
wxString OpCodeStrings[] = {
    OP_CODES};
#undef X

MemoryViewer::MemoryViewer(wxWindow *parent, Memory *memory) : wxWindow(parent, wxID_ANY, wxPoint(0, 0), wxDefaultSize)
{
    wxBoxSizer *columnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *leftColumnSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *memTextSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *programTextSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *rightColumnSizer = new wxBoxSizer(wxVERTICAL);
    wxFont font(wxFontInfo(8).FaceName("Courier New"));

    columnSizer->Add(leftColumnSizer, 3, wxEXPAND);
    columnSizer->Add(rightColumnSizer, 1, wxEXPAND);

    wxPanel *programControls = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    rightColumnSizer->Add(programControls, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxTOP, 10);

    wxPanel *programPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    leftColumnSizer->Add(programPane, 5, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
    programPane->SetSizer(programTextSizer);

    wxArrayString opCodeList;
    wxString memoryString = "";
    char *section = new char[10];
    char *PC = new char[5];
    char *buffer = new char[2];
    char *extBuffer = new char[15];

    for (int i = 0; i < 0x10000; i++)
    {

        bool formatAsOpcode = false;

        if (i < 0x4000)
        {
            sprintf(section, "ROM0: ");
            formatAsOpcode = true;
        }
        else if (i < 0x8000)
        {
            sprintf(section, "ROM%X: ", memory->cart->currentRomBank);
            formatAsOpcode = true;
        }
        else if (i < 0xA000)
        {
            sprintf(section, "VRAM:  ");
        }
        else if (i < 0xC000)
        {
            sprintf(section, "RAM%X: ", memory->cart->currentRamBank);
            formatAsOpcode = true;
        }
        else if (i < 0xD000)
        {
            sprintf(section, "WRAM0: ");
            formatAsOpcode = true;
        }
        else if (i < 0xE000)
        {
            sprintf(section, "WRAM1: ");
            formatAsOpcode = true;
        }
        else if (i < 0xFE00)
        {
            sprintf(section, "ECHO: ");
            formatAsOpcode = true;
        }
        else if (i < 0xFEA0)
        {
            sprintf(section, "OAM:   ");
        }
        else if (i < 0xFF00)
        {
            sprintf(section, "N/A:   ");
        }
        else if (i < 0xFF80)
        {
            sprintf(section, "I/O:   ");
        }
        else if (i < 0xFFFF)
        {
            sprintf(section, "HRAM: ");
            formatAsOpcode = true;
        }
        else
        {
            sprintf(section, "IR:    ");
        }

        // memoryString += section;

        sprintf(PC, "%04X\t", i);
        sprintf(section, "%s%s", section, PC);

        if (formatAsOpcode)
            opCodeList.Add(section + OpCodeStrings[memory->Read(i, true)]);
        else
        {
            sprintf(extBuffer, "%s%02X", section, memory->Read(i, true));
            opCodeList.Add(extBuffer);
        }

        // if (i == 0)
        // {
        //     memoryString += "0000  ";
        // }
        // else if (i % 0x10 == 0)
        // {
        //     sprintf(extBuffer, "\n%04X  ", i);
        //     memoryString += extBuffer;
        // }

        // sprintf(buffer, "%02X ", memory->Read(i, true));
        // memoryString += buffer;
    }

    // Clean up
    delete section;
    delete PC;
    delete buffer;
    delete extBuffer;

    wxListBox *programTextBox = new wxListBox(programPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, opCodeList);
    programTextBox->SetFont(font);
    programTextSizer->Add(programTextBox, 1, wxEXPAND);

    wxPanel *memoryPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    leftColumnSizer->Add(memoryPane, 1, wxEXPAND | wxALL, 10);
    memoryPane->SetSizer(memTextSizer);

    wxTextCtrl *memoryTextBox = new wxTextCtrl(memoryPane, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
    memoryTextBox->SetFont(font);
    memTextSizer->Add(memoryTextBox, 1, wxEXPAND);

    memoryTextBox->AppendText(memoryString);

    SetSizer(columnSizer);
}

MemoryViewer::~MemoryViewer()
{
}