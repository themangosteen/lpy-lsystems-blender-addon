LINDENMAKER ADDON HELP
for Blender 2.7x


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


--- USAGE -----------------------------------------------------------

The Lindenmaker panel can be found in the 3D View Tool Shelf (open via <T> key).

Before using the add-on, a valid L-Py L-system definition file (.lpy) has to be provided.
For information and examples on how to write these, refer to the official L-Py documentation [5].

--- SUPPORTED TURTLE COMMANDS ---

NOTE: The Lindenmaker add-on does not support all turtle interpretation commands allowed by the original L-Py framework. The .lpy file used for L-string production can contain any of the elements related to L-Py, as long as the resulting L-string used for turtle interpretation contains no other than the commands listed in the following.
Other commands than those listed below will typically yield no error but are ignored and have no effect.

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
~   Draw custom object in current turtle context. Parameters: objectname (string, REQUIRED, e.g. "Leaf", must be name of an existing object in the Blender scene), scale (float, optional).

--- THE LINDENMAKER UI PANEL ---

L-Py File Path:
    Path of .lpy file containing L-system definition.
    
Default Turtle Step Size:
    Default length value for 'F' and 'f' commands if no arguments given.
Default Turtle Rotation Angle:
    Default angle in degrees for rotation commands if no argument given.
    
Internode Mesh:
    Name of mesh to be used for drawing internodes via the 'F' command.
    Default is "LindenmakerDefaultInternodeMesh", a cylinder mesh generated at first use.
Default Line Width:
    Default width of internode and node objects drawn via 'F' command.
Default Width Growth Factor:
    Factor by which line width is multiplied via ';' (width increment) command.
    Also used for ','(width decrement) command as 1-(factor-1).
InternodeLengthScale:
    Factor by which move step size is multiplied to yield internode length.
    Used to allow internode length to deviate from step size.
CHECKBOX Draw Nodes:
    If enabled, node objects are drawn, and allows selection of a custom node mesh.
    Default is "LindenmakerDefaultNodeMesh", an icosphere mesh generated at first use.
CHECKBOX Recreate Default Internode / Node Meshes:
    If enabled, recreates "LindenmakerDefaultInternodeMesh" and "LindenmakerDefaultNodeMesh",
    according to attribute values specified via the following fields.
Default Internode Cylinder Vertices:
    Number base circle vertices for default cylinder "LindenmakerDefaultInternodeMesh".
Default Node Icosphere Subdivision:
    Number of subdivision steps for default icosphere "LindenmakerDefaultNodeMesh".

CHECKBOX Force Flat Shading:
    Force flat shading for all parts of the generated structure.
CHECKBOX Single Object (No Hierarchy, Faster)
    If enabled, generate a single object with a single joined mesh. Significantly faster.
    If disabled, generate a branching hierarchy of objects (internode/node meshes are shared).
    
BUTTON Add Mesh via Lindenmayer System
    Do the whole process from L-system definition to graphical interpretation!
    Pass .lpy file to L-Py to produce L-string.
    Apply production rules as many times as specified in file,
    then apply homomorphism substitution rules.
    Finally create a graphical interpretation of the L-string based on the UI options.
    
The following elements can be found in the "Manual L-string Configuration" section.
BUTTON Clear Current L-strings:
    Produce L-string from .lpy file via L-Py.
    Apply production rules as many times as specified in file,
    then apply homomorphism substitution rules.
    No turtle interpretation.
BUTTON Apply One Production Step
    Apply one production step to current L-string, or if none, to the L-system axiom.
    No turtle interpretation.
TEXTBOX L-string for Production:
    Edit via copy/paste.
TEXTBOX Homomorphism (For Interpretation):
    Edit via copy/paste.
    Apply homomorphism rules to current L-string. This is an L-Py feature intended as a
    postproduction step to replace abstract module names by actual interpretation commands.
    In L-Py these rules are preceded by the keyword "homomorphism:" or "interpretation:",
    however this should not be confused with the graphical turtle interpretation!
BUTTON Interpret L-string via Turtle Graphics
    Interpret current L-string via graphical turtle interpretation.
    No production.


--- A NOTE ON MATERIALS ---

TODO


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
    
5.  http://openalea.gforge.inria.fr/dokuwiki/doku.php?id=packages:vplants:lpy:doc
