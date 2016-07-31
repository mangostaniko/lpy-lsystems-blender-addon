import bpy
import re
from math import radians
from mathutils import Vector, Matrix

from lindenmaker import turtle
    
# TODO
# ~(surface)
    
def interpret(lstring, default_length = 1, 
                       default_width = 0.3,
                       default_materialindex = 0, 
                       default_angle = 45):
    """Create graphical representation of L-string via Turtle Interpretation"""
    
    t = turtle.Turtle(default_width, default_materialindex)
    
    # remove all whitespace
    lstring = "".join(lstring.split())
    # apply cut branch commands
    lstring = applyCuts(lstring)
    # split into command symbols with optional parameter lists enclosed by ( and )
    # e.g. "F(230,24)F[+(45)F]F" will yield ['F(230,24)', 'F', '[', '+(45)', 'F', ']', 'F']
    commands = re.findall(r"[^()](?:\([^()]*\))?", lstring)
    
    for cmd in commands:
        
        args = extractArgs(cmd)
        
        if cmd[0] == 'F':
            if len(args) == 2:
                t.move_and_draw(args[0], args[1])
            elif len(args) == 1:
                t.move_and_draw(args[0])
            else:
                t.move_and_draw(default_length)
                # use varargs?
        elif cmd[0] == 'f':
            if len(args) == 1:
                t.move(args[0])
            else:
                t.move(default_length)
            
        elif cmd[0] == '[':
            t.push()
        elif cmd[0] == ']':
            t.pop()
            
        elif cmd[0] == '+':
            if len(args) == 1:
                t.turn(-args[0])
            else:
                t.turn(-default_angle)
        elif cmd[0] == '-':
            if len(args) == 1:
                t.turn(args[0])
            else:
                t.turn(default_angle)
        elif cmd[0] == '&':
            if len(args) == 1:
                t.pitch(-args[0])
            else:
                t.pitch(-default_angle)
        elif cmd[0] == '^':
            if len(args) == 1:
                t.pitch(args[0])
            else:
                t.pitch(default_angle)
        elif cmd[0] == '\\':
            if len(args) == 1:
                t.roll(-args[0])
            else:
                t.roll(-default_angle)
        elif cmd[0] == '/':
            if len(args) == 1:
                t.roll(args[0])
            else:
                t.roll(default_angle)
        elif cmd[0] == '|':
            t.turn(180)
            
        elif cmd[0] == '_':
            if len(args) == 1:
                t.linewidth = args[0]
            else:
                t.linewidth += 0.05
        elif cmd[0] == '!':
            if len(args) == 1:
                t.linewidth = args[0]
            else:
                t.linewidth -= 0.05
            t.linewidth = max(t.linewidth, 0.01)
        elif cmd[0] == ';':
            if len(args) == 1:
                t.materialindex = int(args[0])
            else:
                t.materialindex += 1
        elif cmd[0] == ',':
            if len(args) == 1:
                t.materialindex = int(args[0])
            else:
                t.materialindex -= 1
        t.materialindex = max(0, min(len(bpy.data.materials)-1, t.materialindex), 0)

    t.root.matrix_world *= Matrix.Rotation(radians(-90), 4, 'Y')
    t.root.name = "Root"
    
def applyCuts(lstring):
    """Remove branch segments following a cut command ('%') until their end of branch (i.e. until next unmatched closing bracket or end of string"""
    segments_to_cut = []
    searching_end_of_branch = False
    bracketBalance = 0
    cut_start = cut_end = None
    # find segments to cut
    for i, c in enumerate(lstring):
        if searching_end_of_branch:
            # look for unmatched right bracket (end of branch to cut)
            if c == '[':
                bracketBalance += 1
            elif c == ']':
                bracketBalance -= 1
            if bracketBalance < 0:
                searching_end_of_branch = False
                bracketBalance = 0 
                cut_end = i
                segments_to_cut.append((cut_start, cut_end))
        elif c == '%':
            searching_end_of_branch = True
            cut_start = i
    # no closing bracket found, thus cut until end of string
    if searching_end_of_branch:
        segments_to_cut.append((cut_start, len(lstring)+1))
    # cut segments
    result = lstring
    for (start, end) in segments_to_cut:
        result = result[:start] + '%'*(end-start) + result[end:]
    return result.replace('%', '')

def extractArgs(command):
    argstring_list = re.findall(r"\((.+)\)", command)
    if len(argstring_list) == 0:
        return []
    return [float(arg) for arg in re.split(',', argstring_list[0])]