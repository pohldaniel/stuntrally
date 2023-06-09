/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#ifndef _RENDERTEXTUREENTRY
#define _RENDERTEXTUREENTRY

#include "stringidmap.h"
#include "glwrapper.h"

struct RenderTextureBase
{
	RenderTextureBase() {}
	RenderTextureBase(GLuint newhandle, GLenum newtarget) :
		handle(newhandle), target(newtarget) {}

	GLuint handle;
	GLenum target;
};

struct RenderTextureEntry : public RenderTextureBase
{
	RenderTextureEntry() {}
	RenderTextureEntry(StringId newname, GLuint newhandle, GLenum newtarget) :
		 RenderTextureBase(newhandle,newtarget), name(newname) {}

	StringId name;
};

#endif
