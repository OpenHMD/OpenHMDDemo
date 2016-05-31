/*
* OpenHMD - Free and Open Source API and drivers for immersive technology.
* Copyright (C) 2013 Fredrik Hultin.
* Copyright (C) 2013 Jakob Bornecrantz.
* OpenHMDDemo is designed and written by Joey Ferwerda from Thorworks game development studio.
* Distributed under the Boost 1.0 licence, see LICENSE for full text.
*/
#include "OpenHMD.h"

//OpenHMD helper Functions
OpenHMD::OpenHMD()
{
    ctx = NULL;
    hmd = NULL;
}

int OpenHMD::init()
{
	ctx = ohmd_ctx_create();
	unsigned int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0)
	{
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	printf("num devices: %d\n", num_devices);

	for(unsigned int i = 0; i < num_devices; i++)
	{
		printf("vendor: %s\n", ohmd_list_gets(ctx, i, OHMD_VENDOR));
		printf("product: %s\n", ohmd_list_gets(ctx, i, OHMD_PRODUCT));
		printf("path: %s\n", ohmd_list_gets(ctx, i, OHMD_PATH));
	}

	hmd = ohmd_list_open_device(ctx, 0);

	ohmd_device_geti(hmd, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &ival);
	printf("hres: %i\n", ival);
	ohmd_device_geti(hmd, OHMD_SCREEN_VERTICAL_RESOLUTION, &ival);
	printf("vres: %i\n", ival);

	ohmd_device_getf(hmd, OHMD_SCREEN_HORIZONTAL_SIZE, &fval);
	printf("hsize: %f\n", fval);
	ohmd_device_getf(hmd, OHMD_SCREEN_VERTICAL_SIZE, &fval);
	printf("vsize: %f\n", fval);

	ohmd_device_getf(hmd, OHMD_LENS_HORIZONTAL_SEPARATION, &fval);
	printf("lens seperation: %f\n", fval);
	ohmd_device_getf(hmd, OHMD_LENS_VERTICAL_POSITION, &fval);
	printf("lens vcenter: %f\n", fval);
	ohmd_device_getf(hmd, OHMD_LEFT_EYE_FOV, &fval);
	printf("fov: %f\n", fval);
	ohmd_device_getf(hmd, OHMD_LEFT_EYE_ASPECT_RATIO, &fval);
	printf("aspect: %f\n", fval);

	if(!hmd)
	{
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	return 1;
}

void OpenHMD::exit()
{
    if (ctx)
        ohmd_ctx_destroy(ctx);
}

void OpenHMD::update()
{
	if (ctx)
		ohmd_ctx_update(ctx);
}

Ogre::Quaternion OpenHMD::getQuaternion()
{
    float qu[4];
    ohmd_ctx_update(ctx);
    ohmd_device_getf(hmd, OHMD_ROTATION_QUAT, qu);
    Ogre::Quaternion returnquad = Ogre::Quaternion(qu[3],qu[0],qu[1],qu[2]);
    return returnquad;
}

Ogre::Vector2 OpenHMD::getScreenSize()
{
	Ogre::Vector2 screenSize;
	ohmd_device_geti(hmd, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &ival);
	screenSize.x = ival;
	ohmd_device_geti(hmd, OHMD_SCREEN_VERTICAL_RESOLUTION, &ival);
	screenSize.y = ival;
	return screenSize;
}

void OpenHMD::getVerbose()
{
    float q[4];

    ohmd_ctx_update(ctx);

    ohmd_device_getf(hmd, OHMD_ROTATION_QUAT, q);
    printf("quat: % 4.4f, % 4.4f, % 4.4f, % 4.4f\n", q[0], q[1], q[2], q[3]);
}

std::string OpenHMD::getProduct()
{
	std::string returnDevice;
	///TODO: Should have a way of getting the string information from the current hmd device, workaround for now.
	unsigned int num_devices = ohmd_ctx_probe(ctx);
	for(unsigned int i = 0; i < num_devices; i++)
	{
		returnDevice = ohmd_list_gets(ctx, i, OHMD_PRODUCT);
	}
	return returnDevice;
}

Ogre::Matrix4 OpenHMD::getLeftViewMatrix()
{
    //Get the matrix from OpenHMD and return a Ogre::Matrix4
    float projm[16];
    ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, projm);
    Ogre::Matrix4 oprojm = Ogre::Matrix4(projm[0],projm[1],projm[2],projm[3],
                                         projm[4],projm[5],projm[6],projm[7],
                                         projm[8],projm[9],projm[10],projm[11],
                                         projm[12],projm[13],projm[14],projm[15]);
    return oprojm;
}

Ogre::Matrix4 OpenHMD::getLeftProjectionMatrix()
{
    //Get the matrix from OpenHMD and return a Ogre::Matrix4
    float projm[16];
    ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, projm);
    Ogre::Matrix4 oprojm = Ogre::Matrix4(projm[0],projm[1],projm[2],projm[3],
                                         projm[4],projm[5],projm[6],projm[7],
                                         projm[8],projm[9],projm[10],projm[11],
                                         projm[12],projm[13],projm[14],projm[15]);
    return oprojm;
}

Ogre::Matrix4 OpenHMD::getRightViewMatrix()
{
    //Get the matrix from OpenHMD and return a Ogre::Matrix4
    float projm[16];
    ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, projm);
    Ogre::Matrix4 oprojm = Ogre::Matrix4(projm[0],projm[1],projm[2],projm[3],
                                         projm[4],projm[5],projm[6],projm[7],
                                         projm[8],projm[9],projm[10],projm[11],
                                         projm[12],projm[13],projm[14],projm[15]);
    return oprojm;
}

Ogre::Matrix4 OpenHMD::getRightProjectionMatrix()
{
    //Get the matrix from OpenHMD and return a Ogre::Matrix4
    float projm[16];
    ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, projm);
    Ogre::Matrix4 oprojm = Ogre::Matrix4(projm[0],projm[1],projm[2],projm[3],
                                         projm[4],projm[5],projm[6],projm[7],
                                         projm[8],projm[9],projm[10],projm[11],
                                         projm[12],projm[13],projm[14],projm[15]);

    return oprojm;
}

float OpenHMD::getIPD()
{
    float returnf;
    ohmd_device_getf(hmd, OHMD_EYE_IPD, &returnf);

    return returnf;
}

void OpenHMD::setIPD(float inf)
{
    ohmd_device_setf(hmd, OHMD_EYE_IPD, &inf);
}

bool OpenHMD::isDummy()
{
	ohmd_ctx_probe(ctx);
	if (strcmp(ohmd_list_gets(ctx, 0, OHMD_PRODUCT), "Dummy Device") == 0)
		return true;

	return false;
}
