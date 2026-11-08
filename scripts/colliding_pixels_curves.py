#!/usr/bin/env python3
"""Find simultaneous encounters between two independently specified PixelPanel paths.

Uses only Python's standard library. Models the C++ evaluator: linear motion,
optional perpendicular sine modulation, then mirrored-grid reflection.
A 'duration' sets velocity and sine phase; it does NOT stop the path.
Numerical search: near-encounters are approximate, not proofs of uniqueness.
"""
import argparse
import math
from dataclasses import dataclass

@dataclass
class Path:
    start: tuple
    target: tuple
    mirror: tuple
    duration_ms: int
    amplitude: float
    cycles: float
    width: int
    height: int

    def virtual_target(self):
        result = []
        for target, maximum, mirror in zip(self.target, (self.width-1, self.height-1), self.mirror):
            result.append(mirror*maximum + target if mirror % 2 == 0 else (mirror+1)*maximum-target)
        return tuple(result)

    def velocity(self):
        vt = self.virtual_target()
        return tuple((vt[i]-self.start[i])/(self.duration_ms/1000) for i in (0,1))

    def position(self, ms):
        vx,vy = self.velocity()
        t = ms/1000
        x,y = self.start[0]+vx*t, self.start[1]+vy*t
        length = math.hypot(vx,vy)
        if self.amplitude != 0 and length > 1e-4:
            offset = self.amplitude*math.sin(2*math.pi*self.cycles*ms/self.duration_ms)
            x += -vy/length*offset
            y += vx/length*offset
        return (reflect(x,self.width-1), reflect(y,self.height-1))

def reflect(x, maximum):
    p = x % (2*maximum)
    return p if p <= maximum else 2*maximum-p

def distance(a,b,t):
    ax,ay = a.position(t)
    bx,by = b.position(t)
    return math.hypot(ax-bx,ay-by)

def refine(a,b,lo,hi):
    """Golden-section minimization of separation over a local bracket."""
    r = (math.sqrt(5)-1)/2
    c,d = hi-r*(hi-lo), lo+r*(hi-lo)
    fc,fd = distance(a,b,c),distance(a,b,d)
    for _ in range(65):
        if fc < fd:
            hi,d,fd = d,c,fc
            c=hi-r*(hi-lo)
            fc=distance(a,b,c)
        else:
            lo,c,fc = c,d,fd
            d=lo+r*(hi-lo)
            fd=distance(a,b,d)
    t=(lo+hi)/2
    return t,distance(a,b,t)

def fmt(x):
    return f'{x:g}'

def cpp(path, label):
    s=path.start; target=path.target; mirror=path.mirror
    if path.amplitude or path.cycles:
        if mirror != (0,0):
            return (f'// {label}: sine + reflections: combine helpers in C++ first; '
                    'current makeSineTrajectory does not accept mirror indices.')
        return (f'round_.{label} = makeSineTrajectory({fmt(s[0])}, {fmt(s[1])}, '
                f'{fmt(target[0])}, {fmt(target[1])}, {path.duration_ms}, '
                f'{fmt(path.amplitude)}, {fmt(path.cycles)});')
    if mirror != (0,0):
        return (f'round_.{label} = makeReflectedTrajectory({fmt(s[0])}, {fmt(s[1])}, '
                f'{fmt(target[0])}, {fmt(target[1])}, {mirror[0]}, {mirror[1]}, {path.duration_ms});')
    return (f'round_.{label} = makeTrajectory({fmt(s[0])}, {fmt(s[1])}, '
            f'{fmt(target[0])}, {fmt(target[1])}, {path.duration_ms});')

def main():
    p=argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    for label in ('a','b'):
        p.add_argument(f'--{label}', nargs=2,type=float,required=True,metavar=('X','Y'))
        p.add_argument(f'--target-{label}',nargs=2,type=float,required=True,metavar=('X','Y'))
        p.add_argument(f'--duration-{label}',type=int,required=True,metavar='MS')
        p.add_argument(f'--mirror-{label}',nargs=2,type=int,default=(0,0),metavar=('MX','MY'))
        p.add_argument(f'--sine-{label}',nargs=2,type=float,default=(0,0),metavar=('AMP','CYCLES'))
    p.add_argument('--until',type=int,default=None,metavar='MS',help='Search end time; default maximum duration')
    p.add_argument('--step',type=float,default=2.0,metavar='MS',help='Coarse sampling interval (default 2 ms)')
    p.add_argument('--tolerance',type=float,default=0.15,metavar='PX',help='Maximum distance considered a simultaneous encounter (default 0.15 px)')
    p.add_argument('--width',type=int,default=30)
    p.add_argument('--height',type=int,default=30)
    p.add_argument('--top',type=int,default=10)
    args=p.parse_args()
    if args.width<2 or args.height<2 or args.step<=0 or args.tolerance<0 or args.top<1:
        p.error('invalid dimensions, step, tolerance, or top')
    paths=[]
    for label in ('a','b'):
        start=tuple(getattr(args,label))
        target=tuple(getattr(args,'target_'+label))
        duration=getattr(args,'duration_'+label)
        if duration<=0: p.error('durations must be positive')
        for point in (start,target):
            if not (0<=point[0]<=args.width-1 and 0<=point[1]<=args.height-1):
                p.error('start and target points must be inside the panel')
        paths.append(Path(start,target,tuple(getattr(args,'mirror_'+label)),duration,
                          *getattr(args,'sine_'+label),args.width,args.height))
    a,b=paths
    until=args.until if args.until is not None else max(a.duration_ms,b.duration_ms)
    if until<=0: p.error('--until must be positive')
    n=max(2,math.ceil(until/args.step))
    ts=[until*i/n for i in range(n+1)]
    ds=[distance(a,b,t) for t in ts]
    brackets=[]
    for i in range(1,n):
        if ds[i]<=ds[i-1] and ds[i]<=ds[i+1]:
            brackets.append((ts[i-1],ts[i+1]))
    if ds[0]<=ds[1]: brackets.append((ts[0],ts[1]))
    if ds[-1]<=ds[-2]: brackets.append((ts[-2],ts[-1]))
    minima=[]
    for lo,hi in brackets:
        t,d=refine(a,b,lo,hi)
        if not any(abs(t-old_t)<max(0.01,args.step/10) for old_t,_ in minima):
            minima.append((t,d))
    minima.sort()
    encounters=[(t,d) for t,d in minima if d<=args.tolerance]
    print(f'Paths: A velocity={a.velocity()} px/s; B velocity={b.velocity()} px/s')
    print(f'Searched 0..{until} ms at {until/n:.3f} ms coarse intervals; tolerance={args.tolerance:g} px')
    print(f'Found {len(encounters)} encounter(s) within tolerance; {len(minima)} local separation minima')
    if encounters:
        for t,d in encounters[:args.top]:
            x,y=a.position(t)
            print(f'  ENCOUNTER at {t:.2f} ms, separation {d:.5f} px, near ({x:.2f},{y:.2f})')
        if len(encounters)==1:
            t,d=encounters[0]
            print('\nC++ candidate (collision time rounded to nearest millisecond):')
            print(cpp(a,'a'))
            print(cpp(b,'b'))
            print(f'round_.collisionTimeMs = {round(t)};')
        else:
            print('\nMultiple encounters: this level may have ambiguous scoring opportunities.')
    else:
        print('No simultaneous encounter within tolerance. Closest approaches:')
        for t,d in sorted(minima,key=lambda v:v[1])[:args.top]:
            print(f'  {t:.2f} ms: {d:.4f} px apart')
    print('\nNote: numerical screening can miss very brief encounters. Validate promising levels visually.')

if __name__=='__main__': main()
