LINDENMAKER ADDON HELP
tested with Blender 2.7x, should also work with more recent versions


--- INTRODUCTION ----------------------------------------------------

Lindenmaker is an add-on to visualize structures produced via Lindenmayer systems (L-systems) and Turtle Interpretation in Blender. For a basic introduction to Lindenmayer systems and Turtle Interpretation, refer to the Wikipedia [1] as well as the seminal work by Lindenmayer and Prusinkiewicz [2] and Hanan [3].

The add-on is based upon a custom version of the open source L-Py (Lindenmayer Python) framework by Boudon et al. The original implementation [4] is written in C++ and exposed to Python via Boost::Python, however only supporting Python 2, which is not compatible with the Blender Python API which is based on Python 3.5.

L-Py is used for the derivation of L-strings via application of production rules defined in an .lpy file.
Lindenmaker is responsible for the graphical interpretation (turtle interpretation) and visualisation of the resulting L-string. It reads in an .lpy file, passes it to L-Py to derive the L-string and then produces a hierarchy of objects corresponding to the structure described by the L-string via turtle interpretation, according to parameters specified in the add-on UI within Blender.
Thus the custom version of L-Py has been freed from dependencies to PlantGL which is orginally used for the graphical interpretation of L-strings.


--- INSTALLATION ----------------------------------------------------

1. build L-Py (follow instructions in README.txt in the lpy directory)
2. place lpy.so in <blender scripts dir>/modules
3. place lindenmaker in <blender scripts dir>/addons
4. start Blender and enable the addon from "Preferenecs" > "Add-ons"

Note: To determine the path of <blender scripts dir> on your system, refer to [5].


--- USAGE -----------------------------------------------------------

The Lindenmaker panel can be found in the 3D View Tool Shelf (open via <T> key).

Before using the add-on, a valid L-Py L-system definition file (.lpy) has to be provided.
For information and examples on how to write these, refer to the official L-Py documentation [6].

--- SUPPORTED TURTLE COMMANDS ---

NOTE: The Lindenmaker add-on does not support all turtle interpretation commands allowed by the original L-Py framework. The .lpy file used for L-string production can contain any of the elements related to L-Py, as long as the resulting L-string used for turtle interpretation contains no other than the commands listed in the following.
Other commands than those listed below will typically yield no error but are ignored and have no effect.
Importantly, only single character module names are supported.

The turtle state consists of a 4x4 matrix storing its position and three perpendicular vectors (heading, left and up) defining its orientation (local coordinate system), as well as some attributes related to drawing. Note that drawing attributes can be adjusted via the UI.
Turtle state is altered via commands corresponding to letters in the L-string, as it is read from left to right during turtle interpretation.
Some of these commands take arguments, most of them optional, which if used should be enclosed in parentheses and separated by commas, e.g. F(2,0.5). If no arguments are given, default values are assumed. Details are noted in the following.

F   Move turtle in its heading direction and draw internode object between last and new position. Parameters: length (float, optional), internode radius (float, optional).
f   Move turtle in its heading direction without drawing. Parameters: length (float, optional).

[   Push current turtle state to the stack (and if enabled from ui draw node object).
]   Pop last turtle state from stack and use as current.

+   Turn left around the up vector. Parameters: angle (float, optional)
-   Turn right around the up vector. Parameters: angle (float, optional)
^   Pitch up around the left vector. Parameters: angle (float, optional)
&   Pitch down around the left vector. Parameters: angle (float, optional)
/   Roll left around the heading vector. Parameters: angle (float, optional)
\   Roll right around the heading vector. Parameters: angle (float, optional)
|   Turn halfway around the up vector.

_   Increase internode and node object draw radius by fixed amount. Parameters: radius (float, optional, if given by convention set to this value instead of increment).
!   Decrease internode and node object draw radius by fixed amount. Parameters: radius (float, optional, if given by convention set to this value instead of decrement).
;   Increment material index for internode and node objects. Parameters: materialindex (integer, optional, if given by convention set to this value instead of increment).
,   Decrement material index for internode and node objects. Parameters: materialindex (integer, optional, if given by convention set to this value instead of decrement).

%   Remove remainder of current branch (until next unmatched closing bracket or end of string).
~   Draw custom object in current turtle context. Parameters: objectname (string, REQUIRED, e.g. "Leaf", must be name of an existing object in the Blender scene), scale (float, optional, instead of one scale factor, three separate x y z scale arguments can be given, i.e. ~("Object", scale) or ~("Object", scale_x, scale_y, scale_z)).

--- THE LINDENMAKER UI PANEL ---

L-Py File:
    Path of .lpy file containing L-system definition.
    
Default Turtle Step Size:
    Default length value for 'F' (move turtle and draw) and 'f' (move turtle) commands if no arguments given.
Default Rotation Angle:
    Default angle in degrees for turtle rotation commands if no argument given.
    
Internode:
    Name of mesh to be used for drawing internodes via the 'F' command.
    Default is "LindenmakerDefaultInternodeMesh", a cylinder mesh generated at first use.
Default Line Width:
    Default width of internode and node objects drawn via 'F' command.
Width Growth Factor:
    Factor by which line width is multiplied via ';' (width increment) command.
    Also used for ','(width decrement) command as 1-(factor-1).
Internode Length Scale:
    Factor by which move step size is multiplied to yield internode length.
    Used to allow internode length to deviate from step size.
CHECKBOX Nodes:
    If enabled, the selected node mesh is drawn at branching points.
    If not enabled uses Empty objects if hierarchy is used.
    Default is "LindenmakerDefaultNodeMesh", an icosphere mesh generated at first use.
CHECKBOX Recreate Default Internode / Node Meshes:
    If enabled, recreates "LindenmakerDefaultInternodeMesh" and "LindenmakerDefaultNodeMesh",
    according to attribute values specified via the following fields.
Default Internode Cylinder Vertices:
    Number of base circle vertices for default cylinder "LindenmakerDefaultInternodeMesh".
Default Node Icosphere Subdivision:
    Number of subdivision steps for default icosphere "LindenmakerDefaultNodeMesh".

CHECKBOX Force Flat Shading:
    Force flat shading for all parts of the generated structure.
CHECKBOX Single Object (No Hierarchy, Faster)
    If enabled, generate a single object with a single joined mesh. Significantly faster.
    If disabled, generate a branching hierarchy of objects (internode/node meshes are shared).
CHECKBOX Remove Last Interpretation Result:
    If enabled, the result from the previous interpretation is removed.
    Useful for stepwise production and interpretation, to avoid cluttering the scene.
    
BUTTON Add Mesh via Lindenmayer System
    Do the whole process from L-system definition to graphical interpretation!
    Pass .lpy file to L-Py to produce L-string.
    Apply production rules as many times as specified in file,
    then apply homomorphism substitution rules.
    Finally create a graphical interpretation of the L-string based on the UI options.
    
The following elements can be found in the "Stepwise L-string Production" section.
BUTTON Clear Current L-strings:
    Clear the currently stored L-strings. 
    Note that while the L-system yields just one L-string, two copies of the L-string 
    are stored: One used for production and one used for graphical turtle interpretation.
    See below for details.
BUTTON Apply One Production Step
    Apply one production step to current L-string, or if none, to the L-system axiom.
    No graphical turtle interpretation.
TEXTBOX L-string for Production:
    The produced L-string used for further stepwise production.
    Edit via copy/paste.
TEXTBOX Homomorphism (For Interpretation):
    The same produced L-string but with homomorphism substitution rules applied (if given), 
    used for graphical turtle interpretation.
    Edit via copy/paste.
    "Homomorphism" is a L-Py feature intended as a final postproduction step 
    to replace abstract module names by actual interpretation commands. 
    In L-Py these rules are preceded by the keywords "homomorphism:" or "interpretation:",
    however this should not be confused with the graphical turtle interpretation!
    Once homomorphisms are applied the L-string cant be used for stepwise production,
    thus two L-strings have to be stored.
BUTTON Interpret L-string via Turtle Graphics
    Interpret current L-string via graphical turtle interpretation.
    No production.
BUTTON Produce Step and Interpret
    Apply one production step and interpret (for convenience).


--- MATERIALS ---

Lindenmaker adds material slots to the internode and objects or the respective mesh polygons if no hierarchy used, and assigns existing materials from the global material list (which is usually sorted alphabetically) by the currently set turtle material index!
Thus basically random materials will be applied initially.
The idea is that the important thing is to add material slots which can then be edited afterwards (as specifying materials before interpretation would make it impossible to preview the result).
To avoid unintentional modifications of global materials that are already used in other objects (shared materials might be intended but not always), a "single user" copy of the material can be made from the materials panel.


--- REFERENCES ------------------------------------------------------

1.  https://en.wikipedia.org/wiki/L-system

2.  P. Prusinkiewicz, A. Lindenmayer:
    The Algorithmic Beauty of Plants.
    1990. Springer.
    http://algorithmicbotany.org/papers/#abop
    
3.  J. Hanan:
    Parametric L-Systems and their Application to the Modelling and Visualization of Plants.
    1992. Ph.D. dissertation, University of Regina.
    http://algorithmicbotany.org/papers/hanan.dis1992.html
    
4.  F. Boudon et al.:
    L-Py: An L-System Simulation Framework for Modeling Plant Architecture Development Based on a Dynamic Language.
    2012. Frontiers in Plant Science.
    https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3362793
 
5.  https://www.blender.org/manual/getting_started/installing/configuration/directories.html
    
6.  http://openalea.gforge.inria.fr/dokuwiki/doku.php?id=packages:vplants:lpy:doc
