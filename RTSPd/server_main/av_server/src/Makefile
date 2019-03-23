
LINUX_SRC=/opt/source/Hi3535_SDK_V1.0.4.0/osdrv/kernel/linux-3.4.y
LINUX_INC=$(LINUX_SRC)/include

MODULE_INC=$(LINUX_SRC)/../module/include

EXE = av_server
LIB = 

ROOT = /opt/source/vdm_board/RTSPd


INCLUDE_DIRS = $(ROOT)/libipnc/inc ../inc $(LINUX_INC) $(LINUX_SRC)/../../../mpp/extdrv/gpio

MPI_LIBS = mpi hdmi tde
JPEG_LIBS = jpeg
AUDIO_LIBS = VoiceEngine vqe resampler

LINK_LIBS = brkeyboard rtspserver ipc rtsp pthread m stdc++ raj $(MPI_LIBS) $(JPEG_LIBS) $(AUDIO_LIBS)

include $(ROOT)/build/make.rule
