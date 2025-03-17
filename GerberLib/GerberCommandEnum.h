#ifndef __GERBER_COMMAND_ENUM_H__
#define __GERBER_COMMAND_ENUM_H__


enum EGerberCommandEnum
{
	GC_Unknown,
	GC_G04, // Comment A human readable comment, does not affect the image. 4.1
	GC_MO,  // Mode Sets the unit to mm or inch. 4.2.1
	GC_FS,  // Format specification Sets the coordinate format, e.g.the number of decimals. 4.2.2
	GC_AD,  // Aperture define Defines a template - based aperture, assigns a D code to it. 4.3
	GC_AM,  // Aperture macro Defines a macro aperture template. 4.5
	GC_Dnn, // (nn≥10) Sets the current aperture to D code nn. 4.6
	GC_D01, // Plot operation Outside a region statement D01 creates a draw or arc object with the current aperture.Inside it adds a draw / arc segment to the contour under construction.The current point is moved to draw / arc end point after the creation of the draw / arc. 4.8.2
	GC_D02, // Move operation D02 moves the current point to the coordinate in the command.It does not create an object. 4.8.3
	GC_D03, // Flash operation Creates a flash object with the current aperture.The current point is moved to the flash point. 4.8.4
	GC_G01, // Sets linear / circular mode to linear. 4.7.1
	GC_G02, // Sets linear / circular mode to clockwise circular. 4.7.2
	GC_G03, // Sets linear / circular mode to counterclockwise circular. 4.7.2
	GC_G75, // A G75 must be called before creating the first arc. 4.7.2
	GC_LP,  // Load polarity Loads the polarity object transformation parameter. 4.9.2
	GC_LM,  // Load mirroring Loads the mirror object transformation parameter. 4.9.3
	GC_LR,  // Load rotation Loads the rotation object transformation parameter. 4.9.4
	GC_LS,  // Load scaling Loads the scale object transformation parameter. 4.9.5
	GC_G36, // Starts a region statement which creates a region by defining its contours. 4.10.
	GC_G37, // Ends the region statement. 4.10
	GC_AB,  // Aperture block Opens a block aperture statement and assigns its aperture number or closes a block aperture statement. 4.11
	GC_SR,  // Step and repeat Open or closes a step and repeat statement. 4.11
	GC_TF,  // Attribute on file Set a file attribute. 5.2
	GC_TA,  // Attribute on aperture Add an aperture attribute to the dictionary or modify it. 5.3
	GC_TO,  // Attribute on object Add an object attribute to the dictionary or modify it. 5.4
	GC_TD,  // Attribute delete Delete one or all attributes in the dictionary. 5.5
	GC_M02, // End of file. 4.13
};


#endif // __GERBER_COMMAND_ENUM_H__

