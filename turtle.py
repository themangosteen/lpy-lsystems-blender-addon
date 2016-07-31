import bpy
from math import radians
from mathutils import Vector, Matrix
    
class Turtle:
    
    # TODO
    # - draw custom object (+ expose default cylinder to be changed via ui)
    
    def __init__(self, _linewidth=0.3, _materialindex=0):
        # turtle state consists of a 4x4 matrix and some drawing attributes
        self.mat = Matrix()
        self.linewidth = _linewidth
        self.materialindex = _materialindex
        self.current_parent = None # parent of objects on current branch
        # stack to save and restore turtle state
        self.stack = []
        # init root node
        bpy.ops.object.empty_add(type='ARROWS', radius=0)
        self.root = self.current_parent = bpy.context.object
        bpy.ops.object.select_all(action='DESELECT')
        
    def push(self):
        """Push turtle state to stack and place empty object parent for subsequent cylinders"""
        # push state to stack
        self.stack.append((self.mat.copy(), self.linewidth, self.materialindex, self.current_parent))
        # add empty as new parent for objects on this branch
        bpy.ops.object.empty_add(type='ARROWS', radius=0)
        empty = bpy.context.object
        empty.name = "Branch"
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
        
    def move_and_draw(self, stepsize):
        """Move turtle and draw cylinder between current and new position."""
        bpy.ops.mesh.primitive_cylinder_add(vertices=5,radius=self.linewidth, depth=stepsize)
        cyl = bpy.context.object
        # rotate cylinder mesh to point towards x axis and position origin at base
        bpy.ops.object.mode_set(mode='EDIT', toggle=False)
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.transform.rotate(value=radians(90), axis=(0, 1, 0))
        bpy.ops.transform.translate(value=(stepsize/2,0,0))
        bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
        # set smooth shading
        bpy.ops.object.shade_smooth()
        # set material
        if (bpy.data.materials):
            cyl.data.materials.append(bpy.data.materials[self.materialindex])
        # align cylinder with turtle and put in branching parent hierarchy
        cyl.matrix_world *= self.mat
        self.add_child_to_current_branch_parent(cyl)
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
        if (self.current_parent is None):
            return
        object.parent = self.current_parent
        object.matrix_parent_inverse = self.current_parent.matrix_world.inverted()
        
