bl_info = {
    "name": "Lindenmaker",
    "description": "Lindenmayer systems for Blender via the L-Py framework.",
    "author": "Addon by Nikolaus Leopold. L-Py framework by Frederic Boudon et al.",
    "version": (1, 0),
    "blender": (2, 77, 0),
    "location": "View3D > Add > Mesh",
    "warning": "", # used for warning icon and text in addons panel
    "wiki_url": "",
    "tracker_url": "",
    "support": "COMMUNITY",
    "category": "Add Mesh"
}

from lindenmaker import turtle_interpretation
import lpy
# reload scripts even if already imported, in case they have changed.
# this allows use of operator "Reload Scripts" (key F8)
import imp
imp.reload(turtle_interpretation)
imp.reload(lpy)

import bpy
from math import radians
from mathutils import Vector, Matrix

class LindenmakerPanel(bpy.types.Panel):
    """Lindenmaker Panel"""
    bl_label = "Lindenmaker"
    bl_idname = "OBJECT_PT_lindenmaker"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "world" # TODO put somewhere else??
    
    # get text from open editor file
#    for area in bpy.context.screen.areas:
#        if area.type == 'TEXT_EDITOR':
#            text_editor = area.spaces.active
#            text = text_editor.text.as_string()
#    print(text)

    def draw(self, context):
        layout = self.layout
        
        layout.prop(context.scene, "lpyfile_path")
        
        layout.prop(context.scene, "turtle_step_size")
        layout.prop(context.scene, "turtle_line_width")
        layout.prop(context.scene, "turtle_width_growth_factor")
        layout.prop(context.scene, "turtle_rotation_angle")
        
        box = layout.box()
        box.prop_search(context.scene, "internode_mesh_name", bpy.data, "meshes")
        box.prop(context.scene, "bool_internode_shade_flat")
        box.prop(context.scene, "bool_reset_default_internode_mesh")
        boxcol = box.column()
        boxcol.enabled = context.scene.bool_reset_default_internode_mesh
        boxcol.prop(context.scene, "default_internode_cylinder_vertices")
        
        layout.prop(context.scene, "bool_no_hierarchy")
        
        layout.operator(Lindenmaker.bl_idname, icon='OUTLINER_OB_MESH')
        
        box = layout.box()
        row = box.row()
        row.prop(context.scene, "section_lstring_expanded",
            icon="TRIA_DOWN" if context.scene.section_lstring_expanded else "TRIA_RIGHT",
            icon_only=True, emboss=False)
        row.label(text="Manual L-string Configuration")
        if context.scene.section_lstring_expanded is True:
            row = box.row()
            row.prop(context.scene, "lstring")


class Lindenmaker(bpy.types.Operator):
    """Generate a mesh via a Lindenmayer system""" # tooltip for menu items and buttons.
    bl_idname = "mesh.lindenmaker" # unique identifier for buttons and menu items to reference.
    bl_label = "Add Mesh via Lindenmayer System" # display name in the interface.
    bl_options = {'REGISTER', 'UNDO'} # enable undo for the operator.

    def execute(self, context):
        scene = context.scene
        
        #bpy.ops.object.select_all(action='DESELECT')
        
        # Clear last objects (TODO remove this)
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete()
        for item in bpy.data.meshes:
            if item.users is 0:
                bpy.data.meshes.remove(item)
        for item in bpy.data.materials:
            if item.users is 0:
                bpy.data.materials.remove(item)
        
        # load L-Py framework lsystem specification file (.lpy) and derive lstring
        if scene.lpyfile_path is not "":
            lsys = lpy.Lsystem(scene.lpyfile_path)
            #print("LSYSTEM DEFINITION: {}".format(lsys.__str__()))
            scene.lstring = str(lsys.derive())
            #print("LSYSTEM DERIVATION RESULT: {}".format(context.scene.lstring))
        
        # interpret derived lstring via turtle graphics
        turtle_interpretation.interpret(scene.lstring, scene.turtle_step_size, 
                                                       scene.turtle_line_width,
                                                       scene.turtle_width_growth_factor,
                                                       scene.turtle_rotation_angle,
                                                       default_materialindex=0)
            
        return {'FINISHED'}

def menu_func(self, context):
    self.layout.operator(Lindenmaker.bl_idname, icon='PLUGIN')

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_mesh_add.append(menu_func)
    
    bpy.types.Scene.lpyfile_path = bpy.props.StringProperty(
        name="L-Py File Path", 
        description="Path of .lpy file to be imported", 
        maxlen=1024, subtype='FILE_PATH')
    bpy.types.Scene.turtle_step_size = bpy.props.FloatProperty(
        name="Default Turtle Step Size", 
        default=2.0, 
        min=0.05, 
        max=100.0)
    bpy.types.Scene.turtle_line_width = bpy.props.FloatProperty(
        name="Default Turtle Line Width", 
        default=0.5, 
        min=0.01, 
        max=100.0)
    bpy.types.Scene.turtle_width_growth_factor = bpy.props.FloatProperty(
        name="Default Turtle Width Growth Factor", 
        default=1.05, 
        min=1, 
        max=100.0)
    bpy.types.Scene.turtle_rotation_angle = bpy.props.FloatProperty(
        name="Default Turtle Rotation Angle", 
        default=45.0, 
        min=0.0, 
        max=360.0)
    bpy.types.Scene.internode_mesh_name = bpy.props.StringProperty(
        name="Internode Mesh", 
        description="Name of Mesh to be used for drawing internodes via the F command")
    bpy.types.Scene.default_internode_cylinder_vertices = bpy.props.IntProperty(
        name="Default Internode Cylinder Vertices", 
        default=5, 
        min=3, 
        max=64)
    bpy.types.Scene.bool_internode_shade_flat = bpy.props.BoolProperty(
        name="Internode Flat Shading",
        description="Use flat shading instad of angle based shading for internode mesh",
        default = False)
    bpy.types.Scene.bool_reset_default_internode_mesh = bpy.props.BoolProperty(
        name="Reset Default Internode Mesh",
        description="Recreate the default internode cylinder mesh in case it was modified",
        default = False)
    bpy.types.Scene.bool_no_hierarchy = bpy.props.BoolProperty(
        name="Single Object (No Hierarchy)",
        description="Create a single object instead of a hierarchy tree of objects",
        default = False)
    bpy.types.Scene.lstring = bpy.props.StringProperty(
        name="L-string", 
        description="The L-string resulting from the L-system derivation, to be used for graphical interpretation.")
        
    # properties for UI functionality (like collapsible sections etc.)
    bpy.types.Scene.section_lstring_expanded = bpy.props.BoolProperty(default = False)
    
def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_mesh_add.remove(menu_func)
    
    del bpy.types.Scene.lpyfile_path
    del bpy.types.Scene.turtle_step_size
    del bpy.types.Scene.turtle_line_width
    del bpy.types.Scene.turtle_width_growth_factor
    del bpy.types.Scene.turtle_rotation_angle
    del bpy.types.Scene.internode_mesh_name
    del bpy.types.Scene.default_internode_cylinder_vertices
    del bpy.types.Scene.bool_reset_default_internode_mesh
    del bpy.types.Scene.bool_no_hierarchy
    del bpy.types.Scene.bool_internode_shade_flat
    del bpy.types.Scene.lstring
    
    del bpy.types.Scene.section_lstring_expanded

# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
    register()
    