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

    for (int i = 0; i < 0x1000; i++)
    {
        char *PC = new char[5];
        sprintf(PC, "%04X\t", i);
        opCodeList.Add(PC + OpCodeStrings[memory->Read(i, true)]);
        delete PC;
        if (i == 0)
        {
            memoryString += "0000  ";
        }
        else if (i % 0x10 == 0)
        {
            char *buffer = new char[6];
            sprintf(buffer, "\n%04X  ", i);
            memoryString += buffer;
            delete buffer;
        }
        char *buffer = new char[2];
        sprintf(buffer, "%02X ", memory->Read(i, true));
        memoryString += buffer;
        delete buffer;
    }

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