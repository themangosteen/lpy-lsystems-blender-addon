bl_info = {
    "name": "Lindenmaker",
    "description": "Lindenmayer systems for Blender via the L-Py framework.",
    "author": "Addon by Nikole Leopold. L-Py framework by Frederic Boudon et al.",
    "version": (1, 0),
    "blender": (2, 77, 0),
    "location": "View3D > Add > Mesh",
    "warning": "", # used for warning icon and text in addons panel
    "wiki_url": "",
    "tracker_url": "",
    "support": "COMMUNITY",
    "category": "Add Mesh"
}

import lpy
from lindenmaker import turtle_interpretation
from lindenmaker.turtle_interpretation_error import TurtleInterpretationError
# reload scripts even if already imported, in case they have changed.
# this allows use of operator "Reload Scripts" (key F8)
import imp
imp.reload(lpy)
imp.reload(turtle_interpretation)

import bpy
import os.path
import re
from math import radians
from mathutils import Vector, Matrix

class LindenmakerPanel(bpy.types.Panel):
    """Lindenmaker Panel"""
    bl_label = "Lindenmaker"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_context = "objectmode"
    bl_category = "Lindenmaker"

    def draw(self, context):
        layout = self.layout
        
        layout.prop(context.scene, "lpyfile_path")
        
        col = layout.column() # if placed in column elements are closer to each other
        col.prop(context.scene, "turtle_step_size")
        col.prop(context.scene, "turtle_rotation_angle")
        
        box = layout.box()
        boxlabelcol = box.column()
        boxlabelcol.scale_y = 1.2
        boxlabelrow = boxlabelcol.row()
        boxlabelrow.scale_y = 0.5
        boxlabelrow.prop(context.scene, "section_internode_expanded",
            icon="TRIA_DOWN" if context.scene.section_internode_expanded else "TRIA_RIGHT",
            icon_only=True, emboss=False)
        boxlabelrow.label(text="Internodes and Nodes")
        if context.scene.section_internode_expanded is True:
            boxcol = box.column()
            boxcol.prop_search(context.scene, "internode_mesh_name", bpy.data, "meshes")
            boxcol.prop(context.scene, "turtle_line_width")
            boxcol.prop(context.scene, "turtle_width_growth_factor")
            boxcol.prop(context.scene, "internode_length_scale")
            boxsplit = boxcol.split(1/3)
            boxsplitcol1 = boxsplit.column()
            boxsplitcol1.prop(context.scene, "bool_draw_nodes")
            boxsplitcol2 = boxsplit.column()
            boxsplitcol2.enabled = context.scene.bool_draw_nodes
            boxsplitcol2.prop_search(context.scene, "node_mesh_name", bpy.data, "meshes", text="")
            boxcol.prop(context.scene, "bool_recreate_default_meshes")
            boxcolcol = boxcol.column()
            boxcolcol.enabled = context.scene.bool_recreate_default_meshes
            boxcolcol.alert = context.scene.bool_recreate_default_meshes
            boxcolcol.prop(context.scene, "default_internode_cylinder_vertices")
            boxcolcol.prop(context.scene, "default_node_icosphere_subdivisions")
        
        col = layout.column()
        col.prop(context.scene, "bool_force_shade_flat")
        col.prop(context.scene, "bool_no_hierarchy")
        col.prop(context.scene, "bool_remove_last_interpretation_result")
        
        op_lindenmaker = layout.operator(Lindenmaker.bl_idname, icon='OUTLINER_OB_MESH')
        op_lindenmaker.lstring_production_mode = 'PRODUCE_FULL'
        op_lindenmaker.bool_clear_lstring = True
        op_lindenmaker.bool_interpret_lstring = True
        
        box = layout.box()
        boxlabelcol = box.column()
        boxlabelcol.scale_y = 1.2
        boxlabelrow = boxlabelcol.row()
        boxlabelrow.scale_y = 0.5
        boxlabelrow.prop(context.scene, "section_lstring_expanded",
            icon="TRIA_DOWN" if context.scene.section_lstring_expanded else "TRIA_RIGHT",
            icon_only=True, emboss=False)
        boxlabelrow.label(text="Stepwise L-string Production")
        if context.scene.section_lstring_expanded is True:
            boxcol = box.column()
            
            # button to clear current L-strings
            op_clear = boxcol.operator(Lindenmaker.bl_idname,
                                       text="Clear Current L-strings", 
                                       icon='X')
            op_clear.lstring_production_mode = 'PRODUCE_NONE'
            op_clear.bool_clear_lstring = True
            op_clear.bool_interpret_lstring = False
            
            # button to apply one production step to the current L-string (no interpretation)
            op_produce_step = boxcol.operator(Lindenmaker.bl_idname,
                                              text="Apply One Production Step",
                                              icon='FRAME_NEXT')
            op_produce_step.lstring_production_mode = 'PRODUCE_ONE_STEP'
            op_produce_step.bool_clear_lstring = False
            op_produce_step.bool_interpret_lstring = False
            
            # text field to inspect and edit lstring used for production via copy/paste.
            # this L-string is not used for interpretation (no homomorphism rules applied).
            boxcol.label("L-string for Production (after " + str(context.scene.number_production_steps_done) + " steps):")
            boxcol.prop(context.scene, "lstring_for_production", text="")
            
            # text field to inspect and edit final lstring via copy/paste.
            # this L-string is used for interpretation and has homomorphism rules applied (if any).
            boxcol.label("Homomorphism (for Interpretation):")
            boxcol.prop(context.scene, "lstring_for_interpretation", text="")
            
            # button to do interpretation only (no production)
            op_interpret = boxcol.operator(Lindenmaker.bl_idname,
                                           text="Interpret L-string via Turtle Graphics",
                                           icon='OUTLINER_OB_MESH')
            op_interpret.lstring_production_mode = 'PRODUCE_NONE'
            op_interpret.bool_clear_lstring = False
            op_interpret.bool_interpret_lstring = True
            
            # button to apply one production step and replace the current interpretation result.
            op_interpret_step = boxcol.operator(Lindenmaker.bl_idname,
                                           text="Produce Step and Interpret",
                                           icon='OUTLINER_OB_MESH')
            op_interpret_step.lstring_production_mode = 'PRODUCE_ONE_STEP'
            op_interpret_step.bool_clear_lstring = False
            op_interpret_step.bool_interpret_lstring = True

class Lindenmaker(bpy.types.Operator):
    bl_idname = "mesh.lindenmaker" # unique identifier for buttons and menu items to reference.
    bl_label = "Add Mesh via Lindenmayer System" # display name in the interface.
    bl_description = "Apply Operator" # tooltip
    bl_options = {'REGISTER', 'UNDO'} # enable undo for the operator.

    lstring_production_mode = bpy.props.EnumProperty(
        name="L-string Production Mode",
        description="Mode of operation regarding L-string production.",
        items=(('PRODUCE_FULL', "Produce Full", "Produce L-string from .lpy file via L-Py. "
                                "Apply production rules as many times as specified in file, "
                                "and also do the final homomorphism substitution step.", 0),
               ('PRODUCE_ONE_STEP', "Produce One Step", 
                                    "Apply one production step to current L-string", 1),
               ('PRODUCE_NONE', "Produce None", "Skip L-string production", 2)),
        default='PRODUCE_FULL')
    bool_clear_lstring = bpy.props.BoolProperty(
        name="Clear L-string",
        description="Clear current L-string",
        default=True)
    bool_interpret_lstring = bpy.props.BoolProperty(
        name="Interpret L-string",
        description="Interpret current L-string via graphical turtle interpretation.",
        default=True)    
    
    @classmethod
    def poll(cls, context):
        # operator only available in object mode
        return context.mode == 'OBJECT'

    def execute(self, context):
        scene = context.scene
        
        ##### PRE-OP CLEANUP CONTEXT #####
        
        bpy.ops.object.select_all(action='DESELECT')
        
        # clear scene (for debugging)
        if False: # delete all objects
            bpy.ops.object.select_all(action='SELECT')
            bpy.ops.object.delete()
        if True: # remove unused meshes
            for item in bpy.data.meshes:
                if item.users is 0:
                    bpy.data.meshes.remove(item)
        if False: # remove all materials (including ones currently used)
            for item in bpy.data.materials: 
                item.user_clear()
                bpy.data.materials.remove(item)
                
        if self.bool_clear_lstring:
            scene.lstring_for_production = ""
            scene.lstring_for_interpretation = ""
            scene.number_production_steps_done = 0
            
        ##### LSTRING PRODUCTION #####
        
        if self.lstring_production_mode != 'PRODUCE_NONE':
            
            # load L-Py framework lsystem specification file (.lpy)
            if not os.path.isfile(scene.lpyfile_path):
                self.report({'ERROR_INVALID_INPUT'}, "Input file does not exist! "
                "Select a valid file path in the Lindenmaker options panel in the tool shelf.\n"
                "File not found: {}".format(scene.lpyfile_path))
                return {'CANCELLED'}
            lsys = lpy.Lsystem(scene.lpyfile_path)
            #print("LSYSTEM DEFINITION: {}".format(lsys.__str__()))
            
            # to allow for turtle state queries between L-Py production steps
            # we always have to use derivationLength=1 for Lindenmaker to run the queries
            # before handing back over to L-Py.
            derivationLengthBackup = lsys.derivationLength
            if self.lstring_production_mode == 'PRODUCE_ONE_STEP':
                steps = 1
            else: # PRODUCE_FULL
                steps = derivationLengthBackup
            lsys.derivationLength = 1
            while (steps > 0):
                # use current L-string as axiom unless empty
                if (scene.lstring_for_production != ""):
                    lsys.axiom = lpy.AxialTree(scene.lstring_for_production)
                # derive lstring via production rules (stored as L-Py AxialTree datastructure)
                derivedAxialTree = lsys.derive()
                scene.lstring_for_production = str(derivedAxialTree)
                # substitute occurrences of e.g. ~(Object,4) with ~("Object",4)
                # or ?(P,0,0,0) with ?("P",0,0,0).
                # L-Py strips the quotes, but without them production fails.
                scene.lstring_for_production = re.sub(r'(?<=[~\?]\()(\w*)(?=[,\)])', r'"\1"',
                                                      scene.lstring_for_production)
                scene.number_production_steps_done += 1

                # apply homomorphism substituation step and store result separately.
                # this is an L-Py feature intended as a postproduction step 
                # to replace abstract module names by actual interpretation commands.
                # in L-Py these rules are preceded by keywords "homomorphism:" or "interpretation:",
                # however this should not be confused with the graphical turtle interpretation!
                scene.lstring_for_interpretation = str(lsys.interpret(
                                                       lpy.AxialTree(scene.lstring_for_production)))
                # do a dryrun interpretation without drawing any objects to perform the
                # turtle state queries (via command '?') that will replace the placeholder values
                # in the command arguments with the actual position/heading/up/left vector values.
                # e.g. query ?('P',0,0,0) will become ?('P',Px,Py,Pz) for position vector P.
                # ?(type,x,y,z) can then be used in a production rule.
                try:
                    turtle_interpretation.interpret(scene.lstring_for_interpretation,
                                                    scene.turtle_step_size, 
                                                    scene.turtle_line_width,
                                                    scene.turtle_width_growth_factor,
                                                    scene.turtle_rotation_angle,
                                                    dryrun_nodraw=True)
                except TurtleInterpretationError as e:
                    self.report({'ERROR_INVALID_INPUT'}, str(e))
                    return {'CANCELLED'}
                steps -= 1
                
            lsys.derivationLength = derivationLengthBackup
            #print("LSTRING FOR PRODUCTION: {}".format(context.scene.lstring_for_production))
            #print("LSTRING FOR INTERPRETATION: {}".format(context.scene.lstring_for_interpretation))
        
        ##### GRAPHICAL TURTLE INTERPRETATION #####
        
        if self.bool_interpret_lstring:
            if (scene.bool_remove_last_interpretation_result 
                and scene.last_interpretation_result_objname in bpy.data.objects.keys()):
                delete_hierarchy(bpy.data.objects[scene.last_interpretation_result_objname])
            # interpret derived lstring via turtle graphics
            try:
                turtle_interpretation.interpret(scene.lstring_for_interpretation,
                                                scene.turtle_step_size, 
                                                scene.turtle_line_width,
                                                scene.turtle_width_growth_factor,
                                                scene.turtle_rotation_angle,
                                                default_materialindex=0)
            except TurtleInterpretationError as e:
                self.report({'ERROR_INVALID_INPUT'}, str(e))
                return {'CANCELLED'}
            
        ##### POST-OP CLEANUP #####
            
        # reset operator properties to defaults (needed in case op is called from menu)
        self.lstring_production_mode = 'PRODUCE_FULL'
        self.bool_clear_lstring = True
        self.bool_interpret_lstring = True
        
        return {'FINISHED'}

def menu_func(self, context):
    self.layout.operator(Lindenmaker.bl_idname, icon='PLUGIN')

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_mesh_add.append(menu_func)
    
    bpy.types.Scene.lpyfile_path = bpy.props.StringProperty(
        name="L-Py File", 
        description="Path of .lpy file containing L-system definition, according to the L-Py framework.",
        maxlen=1024, subtype='FILE_PATH')
    bpy.types.Scene.lstring_for_production = bpy.props.StringProperty(
        name="L-string for Production Only", 
        description="The L-string resulting from the L-system productions. No homomorphism rules applied, used for further production only.")
    bpy.types.Scene.lstring_for_interpretation = bpy.props.StringProperty(
        name="L-string for Interpretation", 
        description="The L-string resulting from the L-system productions, with homomorphism rules applied (if any specified).\nUsed for graphical turtle interpretation.")
    bpy.types.Scene.last_interpretation_result_objname = bpy.props.StringProperty(
        name="Last Interpretation Result Object Name", 
        description="Name of the object resulting from the last graphical turtle interpretation.")
    bpy.types.Scene.number_production_steps_done = bpy.props.IntProperty(
        name="Number of production steps already done.", 
        description="Number of production steps already done to produce the current L-string.",
        default=0)
        
    bpy.types.Scene.turtle_step_size = bpy.props.FloatProperty(
        name="Default Turtle Step Size", 
        description="Default length value for 'F' (move turtle and draw) and 'f' (move turtle) commands if no arguments given.",
        default=2.0, 
        min=0.05, 
        max=100.0)
    bpy.types.Scene.turtle_rotation_angle = bpy.props.FloatProperty(
        name="Default Rotation Angle", 
        description="Default angle in degrees for turtle rotation commands if no arguments given.",
        default=45.0, 
        min=0.0, 
        max=360.0)
        
    bpy.types.Scene.internode_mesh_name = bpy.props.StringProperty(
        name="Internode", 
        description="Name of mesh to be used for drawing internodes via the 'F' (move and draw) command.\nDefault is 'LindenmakerDefaultInternodeMesh', a cylinder mesh generated at first use.",
        default="LindenmakerDefaultInternodeMesh")
    bpy.types.Scene.turtle_line_width = bpy.props.FloatProperty(
        name="Default Line Width", 
        description="Default width of internode and node objects drawn via 'F' (move and draw) command.",
        default=0.5, 
        min=0.01, 
        max=100.0)
    bpy.types.Scene.turtle_width_growth_factor = bpy.props.FloatProperty(
        name="Width Growth Factor", 
        description="Factor by which line width is multiplied via ';' (width increment) command.\nAlso used for ',' (width decrement) command as 1-(factor-1).", 
        default=1.05, 
        min=1, 
        max=100.0)
    bpy.types.Scene.internode_length_scale = bpy.props.FloatProperty(
        name="Internode Length Scale", 
        description="Factor by which move step size is multiplied to yield internode length.\nUsed to allow internode length to deviate from step size.", 
        default=1.2,
        min=0.0)
    bpy.types.Scene.bool_draw_nodes = bpy.props.BoolProperty(
        name="Nodes:", 
        description="Draw node objects at branching points.\nOtherwise uses Empty objects if hierarchy is used.",
        default=False)
    bpy.types.Scene.node_mesh_name = bpy.props.StringProperty(
        name="Node Mesh", 
        description="Name of mesh to be used for drawing nodes.\nDefault is 'LindenmakerDefaultNodeMesh', an icosphere mesh generated at first use.",
        default="LindenmakerDefaultNodeMesh")
    bpy.types.Scene.bool_recreate_default_meshes = bpy.props.BoolProperty(
        name="Recreate Default Internode / Node",
        description="Recreates default internode cylinder 'LindenmakerDefaultInternodeMesh' and default node icosphere 'LindenmakerDefaultNodeMesh', according to attribute values, in case they were modified",
        default=False)
    bpy.types.Scene.default_internode_cylinder_vertices = bpy.props.IntProperty(
        name="Internode Cylinder Vertices", 
        description="Number of base circle vertices for default cylinder 'LindenmakerDefaultInternodeMesh'.",
        default=5, 
        min=3, 
        max=64)
    bpy.types.Scene.default_node_icosphere_subdivisions = bpy.props.IntProperty(
        name="Node Icosphere Subdivisions", 
        description="Number of subdivision steps for default icosphere 'LindenmakerDefaultNodeMesh'.",
        default=1, 
        min=1, 
        max=5)
        
    bpy.types.Scene.bool_force_shade_flat = bpy.props.BoolProperty(
        name="Force Flat Shading",
        description="Force flat shading for all parts of the generated structure.",
        default=False)
    bpy.types.Scene.bool_no_hierarchy = bpy.props.BoolProperty(
        name="Single Object (No Hierarchy, Faster)",
        description="Enable to generate a single object with a single joined mesh. Significantly faster.\nDisable to generate a branching hierarchy of objects (internode/node meshes are shared).",
        default=True)
    bpy.types.Scene.bool_remove_last_interpretation_result = bpy.props.BoolProperty(
        name="Remove Last Interpretation Result",
        description="When running the graphical turtle interpretation, the result from the previous interpretation is removed.\nUseful for stepwise production and interpretation, to avoid cluttering the scene.",
        default=False)
        
    bpy.types.Scene.section_internode_expanded = bpy.props.BoolProperty(default = False)
    bpy.types.Scene.section_lstring_expanded = bpy.props.BoolProperty(default = False)
    
def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_mesh_add.remove(menu_func)
    
    del bpy.types.Scene.lpyfile_path
    del bpy.types.Scene.lstring_for_production
    del bpy.types.Scene.lstring_for_interpretation
    del bpy.types.Scene.last_interpretation_result_objname
    del bpy.types.Scene.number_production_steps_done
    
    del bpy.types.Scene.turtle_step_size
    del bpy.types.Scene.turtle_rotation_angle
    
    del bpy.types.Scene.internode_mesh_name
    del bpy.types.Scene.turtle_line_width
    del bpy.types.Scene.turtle_width_growth_factor
    del bpy.types.Scene.internode_length_scale
    del bpy.types.Scene.bool_draw_nodes
    del bpy.types.Scene.node_mesh_name
    del bpy.types.Scene.bool_recreate_default_meshes
    del bpy.types.Scene.default_internode_cylinder_vertices
    del bpy.types.Scene.default_node_icosphere_subdivisions
    
    del bpy.types.Scene.bool_force_shade_flat
    del bpy.types.Scene.bool_no_hierarchy
    del bpy.types.Scene.bool_remove_last_interpretation_result
    
    del bpy.types.Scene.section_internode_expanded
    del bpy.types.Scene.section_lstring_expanded

# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
    register()
    
def delete_hierarchy(obj):
    """Delete an object and all its children"""
    objnames = set([obj.name])

    def get_children_names(obj):
        for child in obj.children:
            objnames.add(child.name)
            if child.children:
                get_children_names(child)

    get_children_names(obj)
    [setattr(bpy.data.objects[name], 'select', True) for name in objnames]
    bpy.ops.object.delete()
