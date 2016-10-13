#ifndef __GLSETTING_H__
#define __GLSETTING_H__

#include "model.h"
#include "macros.h"
#include <opencv2/opencv.hpp>


///<summary>
///Load textrue data from a frame.
///</summary>
///<param name="frame">The frame.</param>
///<param name="texObj">The texture object to be bound.</param>
///<returns>status</returns>
bool LoadTexture(cv::Mat frame, GLuint &texObj);

void loadBackground(float viewport_width, float viewport_height, GLBatch& background_batch);

///<summary>
///Set the OpenGL menus.
///</summary>
void open(Model& model);
void save(Model model);

///<summary>
///Define the OpenGL control.
///</summary>
void GLControl();
#endif