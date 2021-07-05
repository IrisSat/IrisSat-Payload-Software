#ifndef __MT9D111_H__
#define __MT9D111_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

/*!
    \brief                      This function initilizes the camera sensor

    \param[in]                  None

    \return                     0 - Success
                               -1 - Error

    \note
    \warning
*/
long CameraSensorInit(uint8_t camNum);

/*!
    \brief                      Configures sensor in JPEG mode

    \param[in]                  None

    \return                     0 - Success
                               -1 - Error

    \note
    \warning
*/


long StartSensorInJpegMode(int width, int height,uint8_t camNum);
long CameraSensorResolution(int width, int height,uint8_t camNum);
long I2CSelectVar(unsigned char PageNumber, unsigned short VarInt,uint8_t camNum);
long I2CWriteVar(unsigned char PageNumber, unsigned short VarInt,unsigned short VarValue,uint8_t camNum);
long I2CReadVar(unsigned char PageNumber, unsigned short VarInt,unsigned short *VarValue,uint8_t camNum);
long CameraSoftReset(uint8_t camNum);
void DoCapture(uint8_t camNum);
uint32_t CheckJpegSize(uint8_t camNum);
uint32_t checkResolutionWidth(uint8_t camNum);
uint32_t checkResolutionHeight(uint8_t camNum);
uint16_t checkJpegStatus(uint8_t camNum);
void doHandshake(uint8_t camNum);


#define JPEG_HEADER_SIZE	625
extern uint8_t jpegHeader[JPEG_HEADER_SIZE]; //Fill in.

#define JPEG_FOOTER_SIZE	2
extern uint8_t jpegFooter[JPEG_FOOTER_SIZE];
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif //__MT9D111_H__
