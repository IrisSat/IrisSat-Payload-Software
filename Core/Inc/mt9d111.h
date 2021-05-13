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
long CameraSensorInit();

/*!
 \brief                      Configures sensor in JPEG mode

 \param[in]                  None

 \return                     0 - Success
 -1 - Error

 \note
 \warning
 */




long StartSensorInJpegMode(int width, int height);
long CameraSensorResolution(int width, int height);
long I2CSelectVar(unsigned char PageNumber, unsigned short VarInt);
long I2CWriteVar(unsigned char PageNumber, unsigned short VarInt,unsigned short VarValue);
long I2CReadVar(unsigned char PageNumber, unsigned short VarInt,unsigned short *VarValue);

void DoCapture();
long CheckJpegSize();
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //__MT9D111_H__
