bl_info = {
    "name": "Lindenmaker",
    "description": "Lindenmayer systems for Blender via LPY",
    "author": "Nikolaus Leopold",
    "version": (1, 0),
    "blender": (2, 77, 0),
    "location": "View3D > Add > Mesh",
    "warning": "", # used for warning icon and text in addons panel
    "wiki_url": "",
    "tracker_url": "",
    "support": "COMMUNITY",
    "category": "Add Mesh"
}

if "bpy" in locals():
    import imp
    imp.reload(turtle_interpretation)
else:
    from lindenmaker import turtle_interpretation

import bpy
from math import radians
from mathutils import Vector, Matrix


class Lindenmaker(bpy.types.Operator):
    """Generate a mesh via a Lindenmayer system""" # tooltip for menu items and buttons.
    bl_idname = "mesh.lindenmaker" # unique identifier for buttons and menu items to reference.
    bl_label = "Add Mesh via Lindenmayer System" # display name in the interface.
    bl_options = {'REGISTER', 'UNDO'} # enable undo for the operator.

    step_size = bpy.props.FloatProperty(name="Step Size", default=2, min=0.05, max=100)

    def execute(self, context):
        
        #bpy.ops.object.select_all(action='DESELECT')
        
        # DELETE ALL OBJECTS (TODO: remove this)
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete()
        
#       turtle_interpretation.interpret("F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][+F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][-F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]][+F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]];[+F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][-F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]]F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][+F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][-F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]][-F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][+F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]][-F[+F]F[-F][+F[+F]F[-F]]F[+F]F[-F][-F[+F]F[-F]]]]", default_length = 1, default_width = 0.1, default_angle = 35)
                              
        turtle_interpretation.interpret("_(0.1)F(1)[-(60)F(0.3)][+(60)F(0.3)]F(1)[-(60)F(0.3)][+(60)F(0.3)]F(1)[-(60)F(0.3)][+(60)F(0.3)]F(1)[-(60)F(0.3)%F(1)F(1)F(1)F(1)F(1)F(1)B][+(60)F(0.3)%F(1)F(1)F(1)F(1)F(1)F(1)B]F(1)[-(60)X(0)F(1)F(1)F(1)F(1)F(1)B][+(60)X(0)F(1)F(1)F(1)F(1)F(1)B]F(1)[-(60)X(1)F(1)F(1)F(1)F(1)B][+(60)X(1)F(1)F(1)F(1)F(1)B]F(1)[-(60)X(2)F(1)F(1)F(1)B][+(60)X(2)F(1)F(1)F(1)B]F(1)[-(60)X(3)F(1)F(1)B][+(60)X(3)F(1)F(1)B]F(1)[-(60)X(4)F(1)B][+(60)X(4)F(1)B]F(1)[-(60)X(5)B][+(60)X(5)B]A", default_length = 1, default_width = 0.1, default_angle = 35)
       
        # get text from open editor file
        for area in bpy.context.screen.areas:
            if area.type == 'TEXT_EDITOR':
                text_editor = area.spaces.active
                text = text_editor.text.as_string()
        print(text)
            
        return {'FINISHED'}

def menu_func(self, context):
    self.layout.operator(Lindenmaker.bl_idname, icon='PLUGIN')

def register():
    bpy.utils.register_class(Lindenmaker)
    bpy.types.INFO_MT_mesh_add.append(menu_func)
    
def unregister():
    bpy.utils.unregister_class(Lindenmaker)
    bpy.types.INFO_MT_mesh_add.remove(menu_func)


# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
    register()


