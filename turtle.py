import bpy
from math import radians
from mathutils import Vector, Matrix
    
class Turtle:
    
    # TODO
    # - draw custom object
    
    def __init__(self, _linewidth, _materialindex):
        scene = bpy.context.scene
        # turtle state consists of a 4x4 matrix and some drawing attributes
        self.mat = Matrix()
        self.linewidth = _linewidth
        self.materialindex = _materialindex
        self.current_parent = None # parent of objects on current branch
        # stack to save and restore turtle state
        self.stack = []
        # get mesh to use to draw internodes (mesh reuse to save memory)
        if scene.internode_mesh_name in bpy.data.meshes.keys():
            # check for user defined mesh
            self.internode_mesh = bpy.data.meshes[scene.internode_mesh_name]
        else:
            # use default cylinder mesh
            if "LindenmakerDefaultCylinderMesh" in bpy.data.meshes.keys():
                self.default_internode_mesh = bpy.data.meshes["LindenmakerDefaultCylinderMesh"]
                if scene.bool_reset_default_internode_mesh is True:
                    self.default_internode_mesh.user_clear() # also clears fake user
                    self.default_internode_mesh.name = "LindenmakerDefaultCylinderMesh.DEPRECATED"
                    self.create_default_cylinder_mesh(scene.default_internode_cylinder_vertices)
            else:
                self.create_default_cylinder_mesh(scene.default_internode_cylinder_vertices)
            self.internode_mesh = bpy.data.meshes["LindenmakerDefaultCylinderMesh"]
        # init root node
        if bpy.context.scene.bool_no_hierarchy:
            # create empty mesh with no vertices to join with subsequent objects
            bpy.ops.mesh.primitive_plane_add()
            root = bpy.context.object
            root.data.name = "Root"
            root.data.use_auto_smooth = True
            root.data.auto_smooth_angle = radians(85)
            bpy.ops.object.mode_set(mode = 'EDIT')
            bpy.ops.mesh.select_all(action = 'SELECT')
            bpy.ops.mesh.delete()
            bpy.ops.object.mode_set(mode = 'OBJECT')
        else:
            bpy.ops.object.empty_add(type='ARROWS', radius=0)
        self.root = self.current_parent = bpy.context.object
        bpy.ops.object.select_all(action='DESELECT')
        
    def push(self):
        """Push turtle state to stack and place empty object parent for subsequent cylinders"""
        # push state to stack
        self.stack.append((self.mat.copy(), self.linewidth, self.materialindex, self.current_parent))
        if not bpy.context.scene.bool_no_hierarchy:
            # add empty as new parent for objects on this branch
            bpy.ops.object.empty_add(type='ARROWS', radius=0)
            empty = bpy.context.object
            empty.name = "Node"
            empty.matrix_world *= self.mat
            self.add_child_to_current_branch_parent(empty)
            self.current_parent = empty
            bpy.ops.object.select_all(action='DESELECT')
        
    def pop(self):
        """Pop last turtle state from stack and use as current"""
        (self.mat, self.linewidth, self.materialindex, self.current_parent) = self.stack.pop()

    def move(self, stepsize):
        """Move turtle in its heading direction."""
        vec = self.mat * Vector((stepsize,0,0,0))
        self.mat.col[3] += vec 
        
    def move_and_draw(self, stepsize, linewidth=None):
        """Move turtle and draw internode object between current and new position."""
        if linewidth is None:
            linewidth = self.linewidth
        scene = bpy.context.scene
        internode = bpy.data.objects.new('Internode', self.internode_mesh) # reuse internode mesh
        scene.objects.link(internode)
        scene.objects.active = internode
        internode.select = True
        # set shading type
        if scene.bool_internode_shade_flat:
            bpy.ops.object.shade_flat()
        else:
            bpy.ops.object.shade_smooth()
        # create new empty materials if materialindex exceeds length of material list
        # note: the important thing is to create material slots for different parts 
        # of the structure, new materials can be assigned to the created slots later
        while self.materialindex >= len(bpy.data.materials): 
            bpy.ops.material.new()
        # assign material
        # to avoid cluttering the shared internode mesh, link material to current internode object
        # when joining objects (no hierarchy) the related polygons will have the material assigned
        internode.active_material = bpy.data.materials[self.materialindex] # also adds slot if none
        internode.material_slots[0].link = 'OBJECT'
        internode.material_slots[0].material = bpy.data.materials[self.materialindex]
        # add internode to existing structure
        internode.matrix_world *= self.mat
        # set scale
        internode.scale = Vector((stepsize/2*scene.internode_length_scale_factor, linewidth, linewidth))
        # align internode object with turtle
        if scene.bool_no_hierarchy:
            self.root.select = True
            scene.objects.active = self.root
            bpy.ops.object.join()
        else:
            self.add_child_to_current_branch_parent(internode)
        # deselect all and move turtle
        bpy.ops.object.select_all(action='DESELECT')
        self.move(stepsize)
        
    def turn(self, angle_degrees):
        self.mat *= Matrix.Rotation(radians(angle_degrees), 4, 'Z')
    def pitch(self, angle_degrees):
        self.mat *= Matrix.Rotation(radians(angle_degrees), 4, 'Y')
    def roll(self, angle_degrees):
        self.mat *= Matrix.Rotation(radians(angle_degrees), 4, 'X')
        
    def add_child_to_current_branch_parent(self, object):
        if self.current_parent is None:
            return
        object.parent = self.current_parent
        object.matrix_parent_inverse = self.current_parent.matrix_world.inverted()
        
    def create_default_cylinder_mesh(self, vertex_count):
        """Initialize the default cylinder mesh used to draw internodes"""
        bpy.ops.mesh.primitive_cylinder_add(vertices=vertex_count)
        cyl = bpy.context.object
        # rotate cylinder mesh to point towards x axis and position origin at base
        bpy.ops.object.mode_set(mode='EDIT', toggle=False)
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.transform.rotate(value=radians(90), axis=(0, 1, 0))
        bpy.ops.transform.translate(value=(1,0,0))
        bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
        cyl.data.name = "LindenmakerDefaultCylinderMesh"
        # smooth cylinder sides, but not cylinder caps
        cyl.data.use_auto_smooth = True
        cyl.data.auto_smooth_angle = radians(85)
        # assign default mesh and delete object (mesh will persist)
        cyl.data.use_fake_user = True
        bpy.ops.object.delete()
        
