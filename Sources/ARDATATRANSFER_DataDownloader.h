/**
 * @file ARDATATRANSFER_DataDownloader.h
 * @brief libARDataTransfer DataDownloader header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_DATA_DOWNLOADER_PRIVATE_H_
#define _ARDATATRANSFER_DATA_DOWNLOADER_PRIVATE_H_

/**
 * @brief DataDownloader structure
 * @param isInitialized Is set to 1 if DataDownloader initilized else 0
 * @param isCanceled Is set to 1 if DataDownloader Thread is canceled else 0
 * @param isRunning Is set to 1 if DataDownloader Thread is running else 0
 * @param ftp The FTP DataDownloader connection
 * @param localDirectory The local directory where DataDownloader download files
 * @param sem The semaphore to cancel the DataDownloader Thread and its FTP connection
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
typedef struct
{
    int isCanceled;
    int isRunning;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpDataManager;
    char remoteDirectory[ARUTILS_FTP_MAX_PATH_SIZE];
    char localDirectory[ARUTILS_FTP_MAX_PATH_SIZE];
    ARSAL_Sem_t threadSem;
    ARDATATRANSFER_DataDownloader_FileCompletionCallback_t fileCompletionCallback;
    void *fileCompletionArg;

} ARDATATRANSFER_DataDownloader_t;

/**
 * @brief DataDownloader structure
 * @param sum The current sum
 * @param allowedSpace The maximum allowed space
 * @param dir The directory to parse
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
typedef struct
{
    double sum;
    double allowedSpace;
    char dir[ARUTILS_FTP_MAX_PATH_SIZE];

} ARDATATRANSFER_DataDownloader_Fwt_t;

/**
 * @brief Initialize the device DataDownloader (flights data or ride data)
 * @warning This function allocates memory
 * @param manager The pointer of the ADataTransfer Manager
 * @param ftpListManager The ftp list manager
 * @param ftpDataManager The ftp data manager
 * @param localDirectory The path of the local directory where to store data
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_ThreadRun ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_Initialize(ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpListManager, ARUTILS_Manager_t *ftpDataManager, const char *remoteDirectory, const char *localDirectory);

/**
 * @brief Delete an ARDataTransfer DataDownloader connection data
 * @warning This function frees memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
void ARDATATRANSFER_DataDownloader_Clear(ARDATATRANSFER_Manager_t *manager);

/**
 * @brief Remove older data file when exceed the free precent available space allowed
 * @param localPath The local directory path where to check available space
 * @param spacePercent The percent of available space allowed to ride data consuming
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_CheckUsedMemory(const char *localPath, float spacePercent);

/**
 * @brief Compare file extension
 * @param fileName The file name or path to compare with
 * @param ext The extension to compare with
 * @retval 0, -1 or lower -1 or 1 or upper 1
 * @see strcmp C lib
 */
int ARDATATRANSFER_DataDownloader_CompareFileExtension(const char* fileName, const char* ext);

#endif /* _ARDATATRANSFER_DATA_DOWNLOADER_PRIVATE_H_ */
