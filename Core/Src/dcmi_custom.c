/*
 * dcmi_custom.c
 *
 *  Created on: May 21, 2021
 *      Author: howarth3-INS
 */

#include "dcmi_custom.h"
#include "dma.h"
#include "cmsis_os.h"

static void DCMI_DMAXferCplt(DMA_HandleTypeDef *hdma);
static void DCMI_DMAError_custom(DMA_HandleTypeDef *hdma);

volatile uint8_t bufferSwitch = 0;
volatile uint32_t* unusedBuff =0;
volatile uint8_t dcmi_custom_error = 0;

HAL_StatusTypeDef HAL_DCMI_Start_DMA_multi(DCMI_HandleTypeDef *hdcmi, uint32_t DCMI_Mode, uint32_t buffer0, uint32_t buffer1, uint32_t bufferSize, uint32_t imageSize)
{
  /* Initialize the second memory address */
  uint32_t SecondMemAddress = 0;

  /* Check function parameters */
  assert_param(IS_DCMI_CAPTURE_MODE(DCMI_Mode));

  /* Process Locked */
  __HAL_LOCK(hdcmi);

  /* Lock the DCMI peripheral state */
  hdcmi->State = HAL_DCMI_STATE_BUSY;

  /* Enable DCMI by setting DCMIEN bit */
  __HAL_DCMI_ENABLE(hdcmi);

  /* Configure the DCMI Mode */
  hdcmi->Instance->CR &= ~(DCMI_CR_CM);
  hdcmi->Instance->CR |= (uint32_t)(DCMI_Mode);

  /* Set the DMA memory0 conversion complete callback */
  hdcmi->DMA_Handle->XferCpltCallback = DCMI_DMAXferCplt;

  /* Set the DMA error callback */
  hdcmi->DMA_Handle->XferErrorCallback = DCMI_DMAError_custom;

  /* Set the dma abort callback */
  hdcmi->DMA_Handle->XferAbortCallback = NULL;

  /* Reset transfer counters value */
  hdcmi->XferCount = 0;
  hdcmi->XferTransferNumber = 0;
  hdcmi->XferSize = 0;
  hdcmi->pBuffPtr = 0;

  //Clear the CT bit ? https://blog.shirtec.com/2020/09/stm32-hal-dcmi-gocha.html
  hdcmi->Instance->CR &= ~(hdcmi->DMA_Handle->Instance->CR & DMA_SxCR_CT);


//  if (Length <= 0xFFFFU)
//  {
//    /* Enable the DMA Stream */
//    if (HAL_DMA_Start_IT(hdcmi->DMA_Handle, (uint32_t)&hdcmi->Instance->DR, (uint32_t)pData, Length) != HAL_OK)
//    {
//      return HAL_ERROR;
//    }
//  }
//  else /* DCMI_DOUBLE_BUFFER Mode */
//  {
    /* Set the DMA memory1 conversion complete callback */
    hdcmi->DMA_Handle->XferM1CpltCallback = DCMI_DMAXferCplt;

//    /* Initialize transfer parameters */
    hdcmi->XferCount = 1;
    hdcmi->XferSize = bufferSize/2; //Each transfer is buffer size...
    hdcmi->pBuffPtr = buffer0;
//
//    /* Get the number of buffer */
//      hdcmi->XferSize = bufferSioze; // We have 2 even buffers of half Length. //Don't need this...
      hdcmi->XferCount = imageSize/bufferSize;

//
//    /* Update DCMI counter  and transfer number*/
    hdcmi->XferCount = (hdcmi->XferCount - 2U); //Now count is 0???? why?
    hdcmi->XferTransferNumber = hdcmi->XferCount;

    /* Update second memory address */
    SecondMemAddress = (uint32_t)(buffer0 + (4 * hdcmi->XferSize));

    /* Start DMA multi buffer transfer */
    //Start transfer with our 2 buffers.
    if (HAL_DMAEx_MultiBufferStart_IT(hdcmi->DMA_Handle, (uint32_t)&hdcmi->Instance->DR, (uint32_t)buffer0, SecondMemAddress, hdcmi->XferSize) != HAL_OK)
    {
      return HAL_ERROR;
    }
//  }

  /* Enable Capture */
  hdcmi->Instance->CR |= DCMI_CR_CAPTURE;

  /* Release Lock */
  __HAL_UNLOCK(hdcmi);

  /* Return function status */
  return HAL_OK;
}

static void DCMI_DMAError_custom(DMA_HandleTypeDef *hdma)
{
  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  if (hdcmi->DMA_Handle->ErrorCode != HAL_DMA_ERROR_FE)
  {
    /* Initialize the DCMI state*/
    hdcmi->State = HAL_DCMI_STATE_READY;

    /* Set DCMI Error Code */
    hdcmi->ErrorCode |= HAL_DCMI_ERROR_DMA;
  }

  /* DCMI error Callback */
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
  /*Call registered DCMI error callback*/
  hdcmi->ErrorCallback(hdcmi);
#else
  HAL_DCMI_ErrorCallback(hdcmi);
#endif /* USE_HAL_DCMI_REGISTER_CALLBACKS */

}

static void DCMI_DMAXferCplt(DMA_HandleTypeDef *hdma)
{
  uint32_t tmp = 0;

  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  if(bufferSwitch == 1){
	  //This means the user has not finished using the buffer  but we have already switched to it :(
	  //Call error to let user know that their data is potentially bad.
	  hdcmi->ErrorCallback(hdcmi);
	  dcmi_custom_error = 42;

  }
  //We shouldn't need to update any mem addresses. Since i think it hsould do this automatically.
//  //We still have transfers to go.
//  if (hdcmi->XferCount != 0)
//  {
//    /* Update memory 0 address location */
//    tmp = ((hdcmi->DMA_Handle->Instance->CR) & DMA_SxCR_CT);
//    if (((hdcmi->XferCount % 2) == 0) && (tmp != 0))
//    {
//      tmp = hdcmi->DMA_Handle->Instance->M0AR;
//      HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (tmp + (8 * hdcmi->XferSize)), MEMORY0);
//      hdcmi->XferCount--;
//      unusedBuff
//    }
//    /* Update memory 1 address location */
//    else if ((hdcmi->DMA_Handle->Instance->CR & DMA_SxCR_CT) == 0)
//    {
//      tmp = hdcmi->DMA_Handle->Instance->M1AR;
//      HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (tmp + (8 * hdcmi->XferSize)), MEMORY1);
//      hdcmi->XferCount--;
//    }
//  }
//  /* Update memory 0 address location */
//  else if ((hdcmi->DMA_Handle->Instance->CR & DMA_SxCR_CT) != 0)
//  {
//    hdcmi->DMA_Handle->Instance->M0AR = hdcmi->pBuffPtr;
//  }
//  /* Update memory 1 address location */
//  else if ((hdcmi->DMA_Handle->Instance->CR & DMA_SxCR_CT) == 0)
//  {
//    tmp = hdcmi->pBuffPtr;
//    hdcmi->DMA_Handle->Instance->M1AR = (tmp + (4 * hdcmi->XferSize));
//    hdcmi->XferCount = hdcmi->XferTransferNumber;
//  }

  tmp = ((hdcmi->DMA_Handle->Instance->CR) & DMA_SxCR_CT);
  if(tmp == 0){
	  unusedBuff = hdcmi->pBuffPtr + (4 * hdcmi->XferSize); //If we are currently writing to buffer 0, then we can save buffer 1 to flash. so mark as unused.
  }
  else if(tmp == 1){

	  unusedBuff = hdcmi->pBuffPtr;
  }
  bufferSwitch = 1;
  /* Check if the frame is transferred */
  if (hdcmi->XferCount == hdcmi->XferTransferNumber)
  {
    /* Enable the Frame interrupt */
    __HAL_DCMI_ENABLE_IT(hdcmi, DCMI_IT_FRAME);

    /* When snapshot mode, set dcmi state to ready */
    if ((hdcmi->Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
    {
      hdcmi->State = HAL_DCMI_STATE_READY;
    }
  }
}

uint8_t bufferSwitched(){
	return bufferSwitch;
}
uint32_t* getFreeBuff(){
	return unusedBuff;
}
void doneBufferHandling(){
	bufferSwitch = 0;
}

uint8_t getDcmiCustomError(){
	return dcmi_custom_error;
}
