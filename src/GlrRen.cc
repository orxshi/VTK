/*=========================================================================

  Program:   Visualization Toolkit
  Module:    GlrRen.cc
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
#include <iostream.h>
#include "GlrRen.hh"
#include "GlrRenW.hh"
#include "GlrProp.hh"
#include "GlrCam.hh"
#include "GlrLgt.hh"
#include "GlrPoly.hh"
#include "GlrTri.hh"
#include "GlrLine.hh"
#include "GlrPnt.hh"
#include "VolRen.hh"

#define MAX_LIGHTS 8

static float amb_light_info[] = {
  AMBIENT, 0.2, 0.2, 0.2,
  LMNULL
  };

vtkGlrRenderer::vtkGlrRenderer()
{
}

// Description:
// Ask actors to build and draw themselves.
int vtkGlrRenderer::UpdateActors()
{
  vtkActor *anActor;
  float visibility;
  vtkMatrix4x4 matrix;
  int count = 0;
 
  // set matrix mode for actors 
  mmode(MVIEWING);

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
      matrix.Transpose();
 
      // insert model transformation 
      pushmatrix();
      multmatrix((float (*)[4])(matrix[0]));
 
      anActor->Render((vtkRenderer *)this);
 
      popmatrix();
      }
    }
  return count;
}

// Description:
// Ask active camera to load its view matrix.
int vtkGlrRenderer::UpdateCameras ()
{
  // update the viewing transformation 
  if (!this->ActiveCamera) return 0;
  
  this->ActiveCamera->Render((vtkRenderer *)this);
  return 1;
}

// Description:
// Internal method temporarily removes lights before reloading them
// into graphics pipeline.
void vtkGlrRenderer::ClearLights (void)
{
  short cur_light;

  // define a lighting model and set up the ambient light.
  // use index 11 for the heck of it. Doesn't matter except for 0.
   
  // update the ambient light 
  amb_light_info[1] = this->Ambient[0];
  amb_light_info[2] = this->Ambient[1];
  amb_light_info[3] = this->Ambient[2];

  lmdef(DEFLMODEL, 11, 0, amb_light_info);
  lmbind(LMODEL, 11);

  // now delete all the old lights 
  for (cur_light = LIGHT0; 
       cur_light < LIGHT0 + MAX_LIGHTS; cur_light++)
    {
    lmbind(cur_light,0);
    }

  this->NumberOfLightsBound = 0;
}

// Description:
// Ask lights to load themselves into graphics pipeline.
int vtkGlrRenderer::UpdateLights ()
{
  vtkLight *light;
  short cur_light;
  float status;
  int count = 0;

  cur_light= this->NumberOfLightsBound + LIGHT0;

  // set the matrix mode for lighting. ident matrix on viewing stack  
  mmode(MVIEWING);
  pushmatrix();

  for ( this->Lights.InitTraversal(); light = this->Lights.GetNextItem(); )
    {

    status = light->GetSwitch();

    // if the light is on then define it and bind it. 
    // also make sure we still have room.             
    if ((status > 0.0)&& (cur_light < (LIGHT0+MAX_LIGHTS)))
      {
      light->Render((vtkRenderer *)this,cur_light);
      lmbind(cur_light, cur_light);
      // increment the current light by one 
      cur_light++;
      count++;
      // and do the same for the mirror source if backlit is on
      // and we aren't out of lights
      if ((this->BackLight > 0.0) && 
	  (cur_light < (LIGHT0+MAX_LIGHTS)))
	{
	lmbind(cur_light,cur_light);
	// if backlighting is on then increment the current light again 
	cur_light++;
	}
      }
    }
  
  this->NumberOfLightsBound = cur_light - LIGHT0;
  
  popmatrix();
  return count;
}
 
// Description:
// Concrete gl render method.
void vtkGlrRenderer::Render(void)
{
  if (this->StartRenderMethod) 
    {
    (*this->StartRenderMethod)(this->StartRenderMethodArg);
    }

  // standard render method 
  this->ClearLights();
  this->DoCameras();
  this->DoLights();
  this->DoActors();
  // clean up the model view matrix set up by the camera 
  mmode(MVIEWING);
  popmatrix();

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
// Create particular type of gl geometry primitive.
vtkGeometryPrimitive *vtkGlrRenderer::GetPrimitive(char *type)
{
  vtkGeometryPrimitive *prim;

  if (!strcmp(type,"polygons"))
      {
      prim = new vtkGlrPolygons;
      return (vtkGeometryPrimitive *)prim;
      }
  if (!strcmp(type,"triangle_strips"))
      {
      prim = new vtkGlrTriangleMesh;
      return (vtkGeometryPrimitive *)prim;
      }
  if (!strcmp(type,"lines"))
      {
      prim = new vtkGlrLines;
      return (vtkGeometryPrimitive *)prim;
      }
  if (!strcmp(type,"points"))
      {
      prim = new vtkGlrPoints;
      return (vtkGeometryPrimitive *)prim;
      }

  return((vtkGeometryPrimitive *)NULL);
}

void vtkGlrRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderer::PrintSelf(os,indent);

  os << indent << "Number Of Lights Bound: " << 
    this->NumberOfLightsBound << "\n";
}


// Description:
// Return center of renderer in display coordinates.
float *vtkGlrRenderer::GetCenter()
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
				/2.0*(float)size[0]);
	this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
				/2.0*(float)size[1]);
	this->Center[1] = this->Center[1]*(491.0/1024.0);
	}
	break;
      default:
	{
	this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
			   /2.0*(float)size[0]);
	this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
			   /2.0*(float)size[1]);
	}
      }
    }
  else
    {
    this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
			    /2.0*(float)size[0]);
    this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
			    /2.0*(float)size[1]);
    }

  return this->Center;
}


// Description:
// Convert display coordinates to view coordinates.
void vtkGlrRenderer::DisplayToView()
{
  float vx,vy,vz;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;

	vy = 2.0 * (this->DisplayPoint[1]*(1024.0/491.0) - 
		    sizey*this->Viewport[1])/ 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
	}
	break;
      default:
	{
	vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;
	vy = 2.0 * (this->DisplayPoint[1] - sizey*this->Viewport[1])/ 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
	}
      }
    }
  else
    {
    vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
      (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;
    vy = 2.0 * (this->DisplayPoint[1] - sizey*this->Viewport[1])/ 
      (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
    }

  vz = this->DisplayPoint[2];

  this->SetViewPoint(vx*this->Aspect[0],vy*this->Aspect[1],vz);
}

// Description:
// Convert view coordinates to display coordinates.
void vtkGlrRenderer::ViewToDisplay()
{
  int dx,dy;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	dx = (int)((this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
		   (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
		   sizex*this->Viewport[0]);
	dy = (int)((this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
		   (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
		   sizey*this->Viewport[1]);
	dy = (int)(dy*(491.0/1024.0));
	}
	break;
      default:
	{
	dx = (int)((this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
		   (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
		   sizex*this->Viewport[0]);
	dy = (int)((this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
		   (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
		   sizey*this->Viewport[1]);
	}
      }
    }
  else
    {
    dx = (int)((this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
	       (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
	       sizex*this->Viewport[0]);
    dy = (int)((this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
	       (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
	       sizey*this->Viewport[1]);
    }

  this->SetDisplayPoint(dx,dy,this->ViewPoint[2]);
}


// Description:
// Is a given display point in this renderer's viewport.
int vtkGlrRenderer::IsInViewport(int x,int y)
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();


  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	int ty = (int)(y*(1023.0/491.0));

	if ((this->Viewport[0]*size[0] <= x)&&
	    (this->Viewport[2]*size[0] >= x)&&
	    (this->Viewport[1]*size[1] <= ty)&&
	    (this->Viewport[3]*size[1] >= ty))
	  {
	  return 1;
	  }
	}
	break;
      default:
	{
	if ((this->Viewport[0]*size[0] <= x)&&
	    (this->Viewport[2]*size[0] >= x)&&
	    (this->Viewport[1]*size[1] <= y)&&
	    (this->Viewport[3]*size[1] >= y))
	  {
	  return 1;
	  }
	}
      }
    }
  else
    {
    if ((this->Viewport[0]*size[0] <= x)&&
	(this->Viewport[2]*size[0] >= x)&&
	(this->Viewport[1]*size[1] <= y)&&
	(this->Viewport[3]*size[1] >= y))
      {
      return 1;
      }
    }
  
  return 0;
}
