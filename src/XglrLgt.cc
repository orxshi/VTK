/*=========================================================================

  Program:   Visualization Toolkit
  Module:    XglrLgt.cc
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include <math.h>
#include "XglrRenW.hh"
#include "XglrRen.hh"
#include "XglrLgt.hh"

// Description:
// Implement base class method.
void vtkXglrLight::Render(vtkLight *lgt, vtkRenderer *ren,int light_index)
{
  this->Render(lgt, (vtkXglrRenderer *)ren,light_index);
}

// Description:
// Actual light render method.
void vtkXglrLight::Render(vtkLight *lgt, vtkXglrRenderer *ren,int light_index)
{
  Xgl_light *lights;
  Xgl_color light_color;
  Xgl_pt_f3d direction;
  float *Color, *Position, *FocalPoint;
  float Intensity;

  // get required info from light
  Intensity = lgt->GetIntensity();
  Color = lgt->GetColor();
  light_color.rgb.r = Intensity * Color[0];
  light_color.rgb.g = Intensity * Color[1];
  light_color.rgb.b = Intensity * Color[2];
  
  FocalPoint = lgt->GetFocalPoint();
  Position   = lgt->GetPosition();
  direction.x = FocalPoint[0] - Position[0];
  direction.y = FocalPoint[1] - Position[1];
  direction.z = FocalPoint[2] - Position[2];

  lights = ren->GetLightArray();
  
  // define the light source
  xgl_object_set(lights[light_index],
		 XGL_LIGHT_TYPE, XGL_LIGHT_DIRECTIONAL,
		 XGL_LIGHT_COLOR, &light_color,
		 XGL_LIGHT_DIRECTION, &direction,
		 NULL);
  
  vtkDebugMacro(<< "Defining front light\n");
  
  // define another mirror light if backlit is on
  if (ren->GetBackLight()) 
    {
    direction.x = -direction.x;
    direction.y = -direction.y;
    direction.z = -direction.z;

    /* define the light source */
    xgl_object_set(lights[light_index+1],
		   XGL_LIGHT_TYPE, XGL_LIGHT_DIRECTIONAL,
		   XGL_LIGHT_COLOR, &light_color,
		   XGL_LIGHT_DIRECTION, &direction,
		   NULL);
  
    vtkDebugMacro(<< "Defining back light\n");
    }
}

