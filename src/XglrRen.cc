/*=========================================================================

  Program:   Visualization Toolkit
  Module:    XglrRen.cc
  Language:  C++
  Date:      04/20/95
  Version:   1.2


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
#include <iostream.h>
#include "XglrProp.hh"
#include "XglrCam.hh"
#include "XglrLgt.hh"
#include "XglrRenW.hh"
#include "XglrRen.hh"
#include "XglrPoly.hh"
#include "XglrTri.hh"
#include "XglrLine.hh"
#include "XglrPnt.hh"
#include "VolRen.hh"


vtkXglrRenderer::vtkXglrRenderer()
{
}

// Description:
// Ask actors to build and draw themselves.
int vtkXglrRenderer::UpdateActors()
{
  vtkActor *anActor;
  float visibility;
  vtkMatrix4x4 matrix;
  int count = 0;
  Xgl_trans model_trans;

  // loop through actors 
  for ( this->Actors.InitTraversal(); anActor = this->Actors.GetNextItem(); )
    {
    // if it's invisible, we can skip the rest 
    visibility = anActor->GetVisibility();

    if (visibility == 1.0)
      {
      count++;
      // build transformation 
      anActor->GetMatrix(matrix);

      // shold we transpose this matrix ?
      matrix.Transpose();
 
      // insert model transformation 
      xgl_object_get(this->Context,
		     XGL_CTX_GLOBAL_MODEL_TRANS, &model_trans);
      xgl_transform_write(model_trans,(float (*)[4])(matrix[0]));

      anActor->Render((vtkRenderer *)this);
      }
    }
  return count;
}

// Description:
// Ask active camera to load its view matrix.
int vtkXglrRenderer::UpdateCameras ()
{
  // update the viewing transformation 
  if (!this->ActiveCamera) return 0;
  
  this->ActiveCamera->Render((vtkRenderer *)this);
  return 1;
}

// Description:
// Ask lights to load themselves into graphics pipeline.
int vtkXglrRenderer::UpdateLights ()
{
  vtkLight *light;
  short cur_light, idx;
  float status;
  int count = 0;
  Xgl_boolean xglr_switches[MAX_LIGHTS];
  Xgl_color   light_color;

  // first get the lights and switched from the context 
  xgl_object_get(this->Context,XGL_3D_CTX_LIGHTS, this->XglrLights);
  xgl_object_get(this->Context,XGL_3D_CTX_LIGHT_SWITCHES, xglr_switches);
  
  // update the ambient light light# 0
  light_color.rgb.r = this->Ambient[0];
  light_color.rgb.g = this->Ambient[1];
  light_color.rgb.b = this->Ambient[2];
  xgl_object_set(this->XglrLights[0],
		 XGL_LIGHT_TYPE, XGL_LIGHT_AMBIENT,
		 XGL_LIGHT_COLOR, &light_color,
		 NULL);
  
  // set all lights off except the ambient light 
  xglr_switches[0] = TRUE;
  for (idx = 1; idx < MAX_LIGHTS; idx++)
    {
    xglr_switches[idx] = FALSE;
    }
  
  cur_light = 1;

  for ( this->Lights.InitTraversal(); light = this->Lights.GetNextItem(); )
    {
    status = light->GetSwitch();

    // if the light is on then define it and bind it. 
    // also make sure we still have room.             
    if ((status > 0.0)&& (cur_light < MAX_LIGHTS))
      {
      light->Render((vtkRenderer *)this,cur_light);
      xglr_switches[cur_light] = TRUE;
      // increment the current light by one 
      cur_light++;
      count++;
      // and do the same for the mirror source if backlit is on
      // and we aren't out of lights
      if ((this->BackLight > 0.0) && 
	  (cur_light < MAX_LIGHTS))
	{
	xglr_switches[cur_light] = TRUE;
	// if backlighting is on then increment the current light again 
	cur_light++;
	}
      }
    }
  
  // now update the context
  xgl_object_set(this->Context,
		 XGL_3D_CTX_LIGHT_SWITCHES, xglr_switches,
		 NULL);

  this->NumberOfLightsBound = cur_light;
  
  return count;
}
 
// Description:
// Concrete starbase render method.
void vtkXglrRenderer::Render(void)
{
  vtkXglrRenderWindow *temp;

  if (this->StartRenderMethod) 
    {
    (*this->StartRenderMethod)(this->StartRenderMethodArg);
    }

  // update our Context first
  temp = (vtkXglrRenderWindow *)this->GetRenderWindow();
  this->Context = *(temp->GetContext());

  // standard render method 
  this->DoCameras();
  this->DoLights();
  this->DoActors();
  if (this->VolumeRenderer)
    {
    this->VolumeRenderer->Render((vtkRenderer *)this);
    }

  if (this->EndRenderMethod) 
    {
    (*this->EndRenderMethod)(this->EndRenderMethodArg);
    }
}

// Description:
// Create particular type of starbase geometry primitive.
vtkGeometryPrimitive *vtkXglrRenderer::GetPrimitive(char *type)
{
  vtkGeometryPrimitive *prim;

  if (!strcmp(type,"polygons"))
      {
      prim = new vtkXglrPolygons;
      return (vtkGeometryPrimitive *)prim;
      }

  if (!strcmp(type,"triangle_strips"))
      {
      prim = new vtkXglrTriangleMesh;
      return (vtkGeometryPrimitive *)prim;
      }
  if (!strcmp(type,"lines"))
      {
      prim = new vtkXglrLines;
      return (vtkGeometryPrimitive *)prim;
      }
  if (!strcmp(type,"points"))
      {
      prim = new vtkXglrPoints;
      return (vtkGeometryPrimitive *)prim;
      }

  return((vtkGeometryPrimitive *)NULL);
}

void vtkXglrRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderer::PrintSelf(os,indent);

  os << indent << "Number Of Lights Bound: " << 
    this->NumberOfLightsBound << "\n";
}

