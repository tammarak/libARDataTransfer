/**
 * @file ARDATATRANSFER_Uploader.c
 * @brief libARDataTransfer Uploader c file.
 * @date 21/05/2014
 * @author david.flattin.ext@parrot.com
 **/

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Print.h>

#include <libARUtils/ARUTILS_Error.h>
#include <libARUtils/ARUTILS_Manager.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_Uploader.h"
#include "libARDataTransfer/ARDATATRANSFER_DataDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Uploader.h"
#include "ARDATATRANSFER_MediasQueue.h"
#include "ARDATATRANSFER_DataDownloader.h"
#include "ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Manager.h"


#define ARDATATRANSFER_DATA_UPLOADER_TAG          "Uploader"

eARDATATRANSFER_ERROR ARDATATRANSFER_Uploader_New (ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpManager, const char *remotePath, const char *localPath, ARDATATRANSFER_Uploader_ProgressCallback_t progressCallback, void *progressArg, ARDATATRANSFER_Uploader_CompletionCallback_t completionCallback, void *completionArg)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    //eARUTILS_ERROR resultUtil = ARUTILS_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "");
    
    if ((manager == NULL) || (ftpManager == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        if (manager->uploader != NULL)
        {
            result = ARDATATRANSFER_ERROR_ALREADY_INITIALIZED;
        }
        else
        {
            manager->uploader = (ARDATATRANSFER_Uploader_t *)calloc(1, sizeof(ARDATATRANSFER_Uploader_t));
            
            if (manager->uploader == NULL)
            {
                result = ARDATATRANSFER_ERROR_ALLOC;
            }
        }
    }
    
    resultSys = ARSAL_Sem_Post(&manager->dataDownloader->threadSem);
    
    if (resultSys != 0)
    {
        result = ARDATATRANSFER_ERROR_SYSTEM;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->uploader->ftpManager = ftpManager;

        strncpy(manager->uploader->remotePath, remotePath, ARUTILS_FTP_MAX_PATH_SIZE);
        manager->uploader->remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';

        strncpy(manager->uploader->localPath, localPath, ARUTILS_FTP_MAX_PATH_SIZE);
        manager->uploader->localPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        
        manager->uploader->progressCallback = progressCallback;
        manager->uploader->progressArg = progressArg;
        manager->uploader->completionCallback = completionCallback;
        manager->uploader->completionArg = completionArg;
    }
        
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_Uploader_Delete (ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "");
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        if (manager->uploader == NULL)
        {
            result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
        }
        else
        {
            free(manager->uploader);
            manager->uploader = NULL;
        }
    }
    
    return result;
}

void* ARDATATRANSFER_Uploader_ThreadRun (void *managerArg)
{
    ARDATATRANSFER_Manager_t *manager = (ARDATATRANSFER_Manager_t *)managerArg;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtil = ARUTILS_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "%x", (int)manager);
    
    if ((manager != NULL) && (manager->uploader !=  NULL))
    {
        resultUtil = ARUTILS_Manager_Ftp_Put(manager->uploader->ftpManager, manager->uploader->remotePath, manager->uploader->localPath, ARDATATRANSFER_Uploader_Ftp_ProgressCallback, manager, FTP_RESUME_TRUE);
        
        if (resultUtil != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
        if (manager->uploader->completionCallback != NULL)
        {
            manager->uploader->completionCallback(manager->uploader->completionArg, result);
        }
    }
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "exiting");
    
    return NULL;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_Uploader_CancelThread (ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtil = ARUTILS_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "");
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        if (manager->uploader == NULL)
        {
            result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
        }
        else
        {
            resultUtil = ARUTILS_Manager_Ftp_Connection_Cancel(manager->uploader->ftpManager);
            if (resultUtil != ARUTILS_OK)
            {
                result = ARDATATRANSFER_ERROR_FTP;
            }
        }
    }
    
    return result;
}

/*****************************************
 *
 *             Private implementation:
 *
 *****************************************/

void ARDATATRANSFER_Uploader_Ftp_ProgressCallback(void* arg, uint8_t percent)
{
    ARDATATRANSFER_Manager_t *manager = (ARDATATRANSFER_Manager_t *)arg;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_DATA_UPLOADER_TAG, "");
    
    if (manager->uploader->progressCallback != NULL)
    {
        manager->uploader->progressCallback(manager->uploader->progressArg, percent);
    }
}