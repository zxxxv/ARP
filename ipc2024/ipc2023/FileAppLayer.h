#pragma once
#include "BaseLayer.h"
#include "pch.h"
#include <afx.h> // For CFile class (Added for file handling using MFC CFile class)
#include <afxcmn.h> // For CProgressCtrl (Added for using progress bar control)

class CFileAppLayer : public CBaseLayer
{
    CString m_FilePath; // Path of the file to be sent or received
    CProgressCtrl* m_ProgressCtrl; // Progress bar control for tracking send progress
    CFile m_SendFile; // File handle used for sending
    CFile m_ReceiveFile;
    // BOOL m_ExceptionOccurred = FALSE; // File handle used for receiving and assembling segments
public:
    CFileAppLayer(char* name);
    virtual ~CFileAppLayer();
    BOOL Receive(unsigned char* frame); // Receives data from the lower layer
    BOOL Send(CString filePath); // Sends file data to the lower layer
    void SetFilePath(CString Path) { m_FilePath = Path; } // Sets the file path to be used for sending
    CString GetFilePath() { return m_FilePath; } // Gets the file path
    void SetHeader(unsigned short type, unsigned long seq_num, unsigned char* data, UINT dataSize); // Sets the file header with fragment information
    void SetProgressBar(CProgressCtrl* p); // Sets the progress bar control for tracking progress

    typedef struct _FILE_APP {
        unsigned long   fapp_totlen;    // Total length of the file (4 bytes)
        unsigned short  fapp_type;      // Data type: 0x00 (file-info), 0x01 (non frag), 0x02 (mid frag) 0x03 (last frag) (2 bytes)
        // 0x00 (file-info) 프레임에는 파일의 경로가 담긴다.
        unsigned char   faa_msg_type;   // Message type (1 byte)
        unsigned char   unused;         // Unused (1 byte)
        unsigned long   fapp_seq_num;   // Fragment sequence number (4 bytes)
        unsigned char   fapp_data[FILE_DATA_MAX_SIZE]; // Fragment data
    } FILE_APP, * PFILE_APP;

    FILE_APP mHeader; // Header structure for file fragments
    static UINT FILE_SEND(LPVOID pParam); // Static function for sending file data in a separate thread
    BOOL StartSendThread();

private:
    bool SendToEthernetLayer(unsigned char* payload_data, int payload_data_len, unsigned short type); // Sends data to the Ethernet layer
    bool FragmentAndReadFile(unsigned long fileLength); // Reads the file in fragments and sends each fragment
};