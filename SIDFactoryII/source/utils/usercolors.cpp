#include "utils/usercolors.h"
#include "foundation/graphics/color.h"
#include "foundation/graphics/viewport.h"


namespace Utility
{	
	Foundation::Color ToColor(UserColor inColor) 
	{ 
		return Foundation::Color(static_cast<unsigned short>(inColor) + 0x80); 
	}

	void SetUserColor(Foundation::Viewport& inViewport, UserColor inUserColor, Foundation::Color inColor)
	{
		inViewport.SetUserColor(static_cast<unsigned char>(inUserColor), inViewport.GetPalette().GetColorARGB(inColor));
	}


	void SetUserColor(Foundation::Viewport& inViewport, UserColor inUserColor, unsigned int inARGB)
	{
		inViewport.SetUserColor(static_cast<unsigned char>(inUserColor), inARGB);
	}
}