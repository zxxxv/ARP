#include "pch.h"
#include "FileAppLayer.h"

CFileAppLayer::CFileAppLayer(char* name) : CBaseLayer(name) { // Constructor initializes member variables
    m_FilePath = ""; // Initialize file path as empty
    mHeader.fapp_totlen = 0;
    mHeader.fapp_type = 0;
    mHeader.fapp_seq_num = 0;
    mHeader.faa_msg_type = 0;
    mHeader.unused = 0;
    memset(mHeader.fapp_data, 0, FILE_DATA_MAX_SIZE); // Initialize data buffer to zero
}

CFileAppLayer::~CFileAppLayer() {
    // m_SendFile이 열려 있다면 닫습니다.
    if (m_SendFile.m_hFile != CFile::hFileNull) {
        m_SendFile.Close();
    }

    // m_ReceiveFile이 열려 있다면 닫습니다.
    if (m_ReceiveFile.m_hFile != CFile::hFileNull) {
        m_ReceiveFile.Close();
    }

    // 필요에 따라 추가적인 자원 해제 작업을 수행할 수 있습니다.
}

BOOL CFileAppLayer::Send(CString filePath) {
    /* Ethernet Layer로 파일을 내려보내는 함수. / FILE_SEND 함수를 통해 쓰레드로 실행된다.
       # parameter
       - CString filePath: Dialog에서 선택한 파일의 경로. FILE_SEND 함수를 통해 멤버변수에서 가져와 입력된다.
       # return value
       - bool
    */
    TRY{
        // Attempt to open the file for reading in binary mode
        if (!m_SendFile.Open(filePath, CFile::modeRead | CFile::typeBinary)) {
#ifdef _DEBUG
            afxDump << "File could not be opened\n"; // Log if file cannot be opened
#endif       
            AfxMessageBox(_T("파일을 열 수 없습니다."));
            return FALSE;
        }

        unsigned long fileLength = (unsigned long)m_SendFile.GetLength(); // Store total file length (up to ~4GB)
        mHeader.fapp_totlen = fileLength; // Set total length of the file in the header
        m_ProgressCtrl->SetRange(0, fileLength / FILE_DATA_MAX_SIZE);
        if (!FragmentAndReadFile(fileLength)) { return FALSE; } // Fragment the file and send each fragment

        m_SendFile.Close(); // Close the file after sending
    }
        CATCH(CFileException, e) {
#ifdef _DEBUG
        afxDump << "File could not be opened " << e->m_cause << "\n"; // Log the cause if the file could not be opened
#endif
        return FALSE;
    }
    END_CATCH

        return TRUE; // Return success
}


BOOL CFileAppLayer::Receive(unsigned char* frame) { // Receives data from the lower layer
    //if (m_ExceptionOccurred) {
    //    return FALSE;  // 이전에 예외가 발생했으면, 추가 처리를 하지 않음
    //}

    PFILE_APP data = (PFILE_APP)frame;
    try {
        // 수신한 frame 종류 (fapp_type) 에 따른 동작
        // WriteFile : 지정된 파일 또는 I/O(입출력) 디바이스에 데이터를 쓰는 함수
        // fapp_type = 0x00 : 파일의 정보 (이름, 타입 등)
        // 파일의 정보 프레임에서 파일 정보 가져와서 파일 생성
        if (data->fapp_type == 0x00) {
            unsigned char* name_buff = new unsigned char[data->fapp_totlen + 1];  // 1바이트 추가하여 널 종결자 공간 확보
            memcpy(name_buff, data->fapp_data, data->fapp_totlen);
            name_buff[data->fapp_totlen] = '\0';
            CString file_info;
            file_info.Format(_T("%s"), name_buff);
            delete[] name_buff;
            if (!m_ReceiveFile.Open(file_info, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary)) { // file_info 에 들어 있는 파일 정보를 바탕으로 파일 open (없을 시 새로 생성)
                CString errorMessage;
                errorMessage.Format(_T("Failed to open file, seq_num: %lu, opening file: %s, Byte Length: %d in FileAppLayer::Receive"), data->fapp_seq_num, file_info, file_info.GetLength());
                AfxMessageBox(errorMessage); // 파일 열기 실패 메시지 출력
                return FALSE;
            }
            try {
            }
            catch (CFileException* e) {
                CString errorMessage;
                errorMessage.Format(_T("Failed to set file length, seq_num: %lu, in FileAppLayer::Receive"), data->fapp_seq_num);
                AfxMessageBox(errorMessage); // 파일 크기 설정 실패 메시지 출력
                e->Delete();
                return FALSE;
            }
        }
        // fapp_type = 0x01 : 단편화 x 파일 (내용 작성 후 바로 종료)
        else if (data->fapp_type == 0x01) {
            m_ReceiveFile.Seek(data->fapp_seq_num * FILE_DATA_MAX_SIZE, CFile::begin); // fapp_seq_num 을 기준으로 파일의 어떤 위치에 내용을 작성할 지 설정
            m_ReceiveFile.Write(data->fapp_data, data->fapp_totlen);
            m_ReceiveFile.Close();
            AfxMessageBox(_T("파일 받기 완료"));
        }
        // fapp_type = 0x02 : 단편화 o 파일 중 중간 프레임 (마지막 프레임 제외 나머지) (fapp_seq_num 에 따라 파일에 내용 작성)
        else if (data->fapp_type == 0x02) {
            m_ReceiveFile.Seek(data->fapp_seq_num * FILE_DATA_MAX_SIZE, CFile::begin);
            m_ReceiveFile.Write(data->fapp_data, FILE_DATA_MAX_SIZE);
        }
        // fapp_type = 0x03 : 단편화 o 파일 중 마지막 프레임 (fapp_seq_num 에 따라 파일에 내용 작성 후 종료)
        else {
            UINT lastFrameLength = static_cast<UINT>(data->fapp_totlen) - static_cast<UINT>(data->fapp_seq_num) * FILE_DATA_MAX_SIZE;
            m_ReceiveFile.Seek(data->fapp_seq_num * FILE_DATA_MAX_SIZE, CFile::begin);
            m_ReceiveFile.Write(data->fapp_data, lastFrameLength);
            m_ReceiveFile.Close();
            AfxMessageBox(_T("파일 받기 완료"));
        }
    }
    catch (CFileException* e) {
        TCHAR errorCause[256];
        e->GetErrorMessage(errorCause, 256);

        // 예외 메시지 출력
        CString errorMessage;
        errorMessage.Format(_T("File I/O error: %s, seq_num: %lu, total length: , in FileAppLayer"), errorCause, data->fapp_seq_num);
        AfxMessageBox(errorMessage);

        e->Delete();
        return FALSE;
    }
    catch (...) {
        AfxMessageBox(_T("An unknown error occurred. seq_num: %lu, in FileAppLayer"), data->fapp_seq_num);
        return FALSE;
    }

    return true;
}


void CFileAppLayer::SetHeader(unsigned short type, unsigned long seq_num, unsigned char* data, UINT dataSize) {
    /* FileApp 헤더를 설정하는 함수
       # parameter
       - unsigned short type: 해당 프레임이 단편화되지 않은 상태인지(0x01), 단편화된 첫번째 프레임인지(0x01), 단편화된 나머지 프레임인지(0x02)를 나타낸다.
       - unsigned long: 해당 프레임이 몇 번째 프레임인지를 나타낸다(시작은 0)
       - unsigned char* data: 전송할 데이터를 나타낸다.
    */
    mHeader.fapp_type = type; // Set the fragment type (e.g., first fragment, subsequent fragment)
    mHeader.fapp_seq_num = seq_num; // Set the fragment sequence number
    memcpy(mHeader.fapp_data, data, dataSize); // Copy the data into the fragment buffer
    if (type == 0x00) mHeader.fapp_totlen = dataSize;
}

void CFileAppLayer::SetProgressBar(CProgressCtrl* p) {
    m_ProgressCtrl = p; // Set the progress bar control for tracking progress
}

/*
 * Function: FILE_SEND
 * Purpose: Placeholder function for sending file data with progress bar updates.
 */
UINT CFileAppLayer::FILE_SEND(LPVOID pParam) {
    /* send 함수를 쓰레드로 돌리기 위해 존재하는 함수이다.
       # parameter : File Layer
    */
    CFileAppLayer* pLayer = (CFileAppLayer*)pParam; // Cast the parameter to CFileAppLayer
    if (pLayer == nullptr) {
        return 1; // Return error if the parameter is null
    }

    CString filePath = pLayer->GetFilePath(); // Get the file path from the layer
    return pLayer->Send(filePath) ? 0 : 1; // Call Send and return the result

}

BOOL CFileAppLayer::StartSendThread() {
    /* send 함수를 쓰레드로 돌리기 위해 존재하는 함수이다. Dialog에서 트리거되어 -> FILE_SEND -> send 순으로 호출된다.
       # return value : bool
    */
    return (AfxBeginThread(FILE_SEND, this) != nullptr) ? TRUE : FALSE;
}

/*
 * Function: SendToEthernetLayer
 * Purpose: Sends the given data to the Ethernet layer.
 */
bool CFileAppLayer::SendToEthernetLayer(unsigned char* payload_data, int payload_data_len, unsigned short type) {
    // 그냥 보기 좋으라고 쓰는 함수이다.
    // Send the data to the Ethernet layer using the lower layer's Send function
    return ((CEthernetLayer*)(this->GetUnderLayer()))->Send(payload_data, payload_data_len, type);
}

/*
 * Function: FragmentAndReadFile
 * Purpose: Reads a file in fragments of 1488 bytes each, starting from byte 0 to byte 1487, then from byte 1488 to 2*1488-1, etc.
 * Updated: No longer opens the file directly, takes fileLength as a parameter.
 */
bool CFileAppLayer::FragmentAndReadFile(unsigned long fileLength) {
    // 프레임 타입 정의
    const unsigned short FRAME_TYPE_PATH = 0x00;
    const unsigned short FRAME_TYPE_UNFRAGMENTED_DATA = 0x01;
    const unsigned short FRAME_TYPE_FRAGMENTED_DATA = 0x02;
    const unsigned short FRAME_TYPE_FRAGMENTED_DATA_LAST = 0x03;

    // 파일 경로를 멤버 변수에서 가져옴
    CString filePath = GetFilePath();
    CString fileName = filePath.Mid(filePath.ReverseFind('\\') + 1);
    const char* fileNameCStr = (LPCTSTR)fileName; // CString을 const char*로 변환
    unsigned int fileNameLength = strlen(fileNameCStr);

    // 0x00 프레임으로 파일 경로 전송
    SetHeader(FRAME_TYPE_PATH, 0, (unsigned char*)fileNameCStr, fileNameLength);
    int frameLength = FILE_HEADER_SIZE + fileNameLength;
    CString errorMessage; //
    errorMessage.Format(_T("%s"), fileNameCStr); // 
    AfxMessageBox(errorMessage); //
    if (!SendToEthernetLayer((unsigned char*)&mHeader, frameLength, FILE_LAYER_IDENTIFIER)) {
        return false; // 파일 경로 전송 실패 시 false 반환
    }
    // 파일 길이가 최대 데이터 크기 이하인 경우 (단편화되지 않은 경우)
    if (fileLength <= FILE_DATA_MAX_SIZE) {
        char buffer[FILE_DATA_MAX_SIZE];
        UINT bytesRead = m_SendFile.Read(buffer, fileLength); // 파일 전체를 버퍼에 읽어들임
        mHeader.fapp_totlen = bytesRead;
        if (bytesRead > 0) {
            // 0x01 프레임으로 파일 데이터 전송
            SetHeader(FRAME_TYPE_UNFRAGMENTED_DATA, 0, (unsigned char*)buffer, bytesRead);
            int frameLength = FILE_HEADER_SIZE + bytesRead;
            return SendToEthernetLayer((unsigned char*)&mHeader, frameLength, FILE_LAYER_IDENTIFIER);
        }
        return false; // 파일에서 데이터를 읽지 못한 경우
    }
    mHeader.fapp_totlen = fileLength;

    // 단편화된 경우
    unsigned long offset = 0; // 파일 읽기의 시작 오프셋
    unsigned long seq_num = 0; // 각 프래그먼트의 시퀀스 번호
    char buffer[FILE_DATA_MAX_SIZE];
    UINT bytesRead = 0;

    while (offset < fileLength) {
        m_SendFile.Seek(offset, CFile::begin); // 현재 오프셋으로 파일 포인터 이동
        bytesRead = m_SendFile.Read(buffer, FILE_DATA_MAX_SIZE); // 최대 크기만큼 데이터 읽기
        if (bytesRead > 0) { // 0보다 크면 데이터가 존재함
            unsigned short fragType;
            if ((offset + bytesRead) >= fileLength) {
                // 마지막 프래그먼트인 경우
                fragType = FRAME_TYPE_FRAGMENTED_DATA_LAST; // 0x03
            }
            else {
                // 중간 또는 첫 번째 프래그먼트인 경우
                fragType = FRAME_TYPE_FRAGMENTED_DATA; // 0x02
            }
            SetHeader(fragType, seq_num++, (unsigned char*)buffer, bytesRead); // 헤더 설정

            int frameLength = FILE_HEADER_SIZE + bytesRead;
            if (!SendToEthernetLayer((unsigned char*)&mHeader, frameLength, FILE_LAYER_IDENTIFIER)) {
                return false; // 전송 실패 시 false 반환
            }
            m_ProgressCtrl->SetPos(seq_num);
        }
        else {
            break; // 더 이상 읽을 데이터가 없는 경우 루프 종료
        }
        offset += bytesRead; // 다음 오프셋으로 이동
    }
    AfxMessageBox(_T("파일 전송 완료"));
    //m_ProgressCtrl->SetPos(0);
    return true; // 전송 성공
}

