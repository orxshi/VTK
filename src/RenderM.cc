/*=========================================================================

  Program:   Visualization Toolkit
  Module:    RenderM.cc
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
#include <stdlib.h>
#include <iostream.h>
#include <string.h>
#include "RenderM.hh"

#ifdef USE_KGLR
#include "KglrRenW.hh"
#endif

#ifdef USE_SBR
#include "SbrRenW.hh"
#endif

#ifdef USE_GLR
#include "GlrRenW.hh"
#endif

#ifdef USE_OGLR
#include "OglrRenW.hh"
#endif

#ifdef USE_XGLR
#include "XglrRenW.hh"
#endif

vtkRenderMaster::vtkRenderMaster()
{
}

// Description:
// Create named renderer type.
vtkRenderWindow *vtkRenderMaster::MakeRenderWindow(char *type)
{

#ifdef USE_KGLR
  if (!strncmp("kglr",type,4))
    {
    vtkKglrRenderWindow *ren;
    ren = new vtkKglrRenderWindow;
    return (vtkRenderWindow *)ren;
    }
#endif

#ifdef USE_SBR
  if (!strncmp("sbr",type,4))
    {
    vtkSbrRenderWindow *ren;
    ren = new vtkSbrRenderWindow;
    return (vtkRenderWindow *)ren;
    }
#endif

#ifdef USE_GLR
  if (!strncmp("glr",type,3))
    {
    vtkGlrRenderWindow *ren;
    ren = new vtkGlrRenderWindow;
    return (vtkRenderWindow *)ren;
    }
#endif

#ifdef USE_OGLR
  if (!strncmp("oglr",type,4))
    {
    vtkOglrRenderWindow *ren;
    ren = new vtkOglrRenderWindow;
    return (vtkRenderWindow *)ren;
    }
#endif

#ifdef USE_XGLR
  if (!strncmp("xglr",type,4))
    {
    vtkXglrRenderWindow *ren;
    ren = new vtkXglrRenderWindow;
    return (vtkRenderWindow *)ren;
    }
#endif

  vtkErrorMacro(<<"RenderMaster Error: unable to return render window.\n");
  return (vtkRenderWindow *)NULL;
}

// Description:
// Create renderer based on environment variable VTK_RENDERER. If VTK_RENDERER
// not defined, then use default renderer kglr.
vtkRenderWindow *vtkRenderMaster::MakeRenderWindow(void)
{
  char *temp;
  
  // if nothing is set then try kglr
  temp = getenv("VTK_RENDERER");
  if (!temp) temp = "kglr";

  return (this->MakeRenderWindow(temp));
}

void vtkRenderMaster::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);
}

