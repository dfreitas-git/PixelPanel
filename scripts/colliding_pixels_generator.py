#!/usr/bin/env python3
"""Search collision endpoints for two reflected PixelPanel trajectories.

No third-party dependencies. Coordinates range from 0 to width-1/height-1.
Mirrors use the same convention as CollidingPixelsScene::makeReflectedTrajectory.
"""
import argparse
import math
from dataclasses import dataclass

EPS = 1e-10


def reflect(u, maximum):
    period = 2.0 * maximum
    p = u % period
    return p if p <= maximum else period - p


def virtual_target(target, maximum, mirror):
    return mirror * maximum + target if mirror % 2 == 0 else (mirror + 1) * maximum - target


@dataclass(frozen=True)
class Path:
    start: tuple
    target: tuple
    mirror: tuple
    velocity: tuple  # pixels per second
    duration: float
    width: int
    height: int

    def position(self, fraction):
        return tuple(reflect(self.start[i] + (self.target[i] - self.start[i]) * fraction,
                             (self.width - 1, self.height - 1)[i]) for i in (0, 1))

    def breaks(self):
        """Normalized times of every wall reflection, including endpoints."""
        result = [0.0, 1.0]
        for axis, maximum in enumerate((self.width - 1, self.height - 1)):
            a, b = self.start[axis], self.target[axis]
            if abs(b - a) < EPS:
                continue
            lo, hi = sorted((a, b))
            for k in range(math.floor(lo / maximum) + 1, math.ceil(hi / maximum)):
                t = (k * maximum - a) / (b - a)
                if EPS < t < 1.0 - EPS:
                    result.append(t)
        return result


def make_path(start, collision, mirror, duration, width, height):
    target = tuple(virtual_target(collision[i], (width - 1, height - 1)[i], mirror[i])
                   for i in (0, 1))
    velocity = tuple((target[i] - start[i]) / duration for i in (0, 1))
    return Path(start, target, mirror, velocity, duration, width, height)


def unique_sorted(values):
    out = []
    for value in sorted(values):
        if not out or value - out[-1] > EPS:
            out.append(value)
    return out


def min_distance(a, b, t_start, t_end):
    """Exact minimum physical separation over a normalized-time interval.

    Between consecutive wall reflections each coordinate is affine, so the
    squared distance is a quadratic. Its minimum occurs at an endpoint or
    the stationary point of that quadratic.
    """
    if t_end < t_start:
        raise ValueError('invalid time interval')
    knots = unique_sorted([t_start, t_end] +
                          [t for t in a.breaks() + b.breaks()
                           if t_start < t < t_end])
    best = math.inf
    best_t = t_start
    for lo, hi in zip(knots, knots[1:]):
        pa0, pb0 = a.position(lo), b.position(lo)
        pa1, pb1 = a.position(hi), b.position(hi)
        r0 = (pa0[0] - pb0[0], pa0[1] - pb0[1])
        dr = ((pa1[0] - pb1[0]) - r0[0], (pa1[1] - pb1[1]) - r0[1])
        denom = dr[0] ** 2 + dr[1] ** 2
        candidates = [0.0, 1.0]
        if denom > EPS:
            candidates.append(max(0.0, min(1.0, -(r0[0]*dr[0] + r0[1]*dr[1]) / denom)))
        for f in candidates:
            distance = math.hypot(r0[0] + f*dr[0], r0[1] + f*dr[1])
            if distance < best:
                best, best_t = distance, lo + (hi - lo)*f
    if len(knots) == 1:
        pa, pb = a.position(t_start), b.position(t_start)
        return math.dist(pa, pb), t_start
    return best, best_t


def parse_pair(values):
    return (float(values[0]), float(values[1]))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--a', nargs=2, type=float, required=True, metavar=('X', 'Y'), help='Pixel A starting position')
    parser.add_argument('--b', nargs=2, type=float, required=True, metavar=('X', 'Y'), help='Pixel B starting position')
    parser.add_argument('--mirror-a', nargs=2, type=int, required=True, metavar=('MX', 'MY'))
    parser.add_argument('--mirror-b', nargs=2, type=int, required=True, metavar=('MX', 'MY'))
    parser.add_argument('--time', type=int, required=True, metavar='MS', help='Intended collision time in milliseconds')
    parser.add_argument('--width', type=int, default=30)
    parser.add_argument('--height', type=int, default=30)
    parser.add_argument('--window', type=int, default=200, metavar='MS', help='Allowed final approach window (default: 200 ms)')
    parser.add_argument('--clearance', type=float, default=2.0, metavar='PX', help='Required separation before final window (default: 2 px)')
    parser.add_argument('--top', type=int, default=10, metavar='N', help='Number of candidates to print')
    parser.add_argument('--check', nargs=2, type=float, metavar=('X', 'Y'), help='Validate only this collision endpoint')
    args = parser.parse_args()
    if args.width < 2 or args.height < 2 or args.time <= 0 or args.window < 0 or args.window >= args.time or args.clearance < 0 or args.top < 1:
        parser.error('invalid dimensions, time, window, clearance or top count')
    start_a, start_b = parse_pair(args.a), parse_pair(args.b)
    mirrors = (tuple(args.mirror_a), tuple(args.mirror_b))
    for start in (start_a, start_b):
        if not (0 <= start[0] <= args.width-1 and 0 <= start[1] <= args.height-1):
            parser.error('starting positions must be inside the panel')
    duration = args.time / 1000.0
    cutoff = 1.0 - args.window / args.time
    endpoints = [tuple(args.check)] if args.check is not None else [
        (float(x), float(y)) for y in range(args.height) for x in range(args.width)]
    if args.check and not (0 <= args.check[0] <= args.width-1 and 0 <= args.check[1] <= args.height-1):
        parser.error('check endpoint must be inside the panel')
    results = []
    for endpoint in endpoints:
        a = make_path(start_a, endpoint, mirrors[0], duration, args.width, args.height)
        b = make_path(start_b, endpoint, mirrors[1], duration, args.width, args.height)
        clearance, closest_t = min_distance(a, b, 0.0, cutoff)
        # Separately check for an exact earlier intersection, including inside
        # the final approach window. Exclude only the intended endpoint itself.
        early_min, _ = min_distance(a, b, 0.0, 1.0 - 1.0 / args.time)
        exact_early = early_min < 1e-7
        valid = clearance >= args.clearance and not exact_early
        results.append((valid, clearance, closest_t, endpoint, a, b, exact_early))
    valid_results = sorted((r for r in results if r[0]), key=lambda r: (-r[1], r[3][1], r[3][0]))
    print(f'Checked {len(results)} endpoint(s); {len(valid_results)} passed the {args.clearance:g} px clearance requirement')
    print(f'Final approach window: {args.window} ms; tested continuously over 0..{args.time-args.window} ms')
    selected = results if args.check else valid_results[:args.top]
    if not selected:
        print('No valid endpoints. Try reducing --clearance or --window, or changing the starting positions/mirrors.')
    for idx, (valid, clearance, closest_t, endpoint, a, b, exact_early) in enumerate(selected, 1):
        print(f'\nCandidate {idx}: collision=({endpoint[0]:g}, {endpoint[1]:g})  '
              f'{"PASS" if valid else "REJECT"}  min separation={clearance:.3f} px at {closest_t*args.time:.1f} ms')
        if exact_early:
            print('  WARNING: earlier exact intersection detected')
        print(f'  A velocity=({a.velocity[0]:.3f}, {a.velocity[1]:.3f}) px/s; '
              f'B velocity=({b.velocity[0]:.3f}, {b.velocity[1]:.3f}) px/s')
        def fmt(v):
            return f'{v:g}'
        for label, start, mirror in [('a', start_a, mirrors[0]), ('b', start_b, mirrors[1])]:
            print(f'  round_.{label} = makeReflectedTrajectory('
                  f'{fmt(start[0])}, {fmt(start[1])}, {fmt(endpoint[0])}, {fmt(endpoint[1])}, '
                  f'{mirror[0]}, {mirror[1]}, {args.time});')
        print(f'  round_.collisionTimeMs = {args.time};')
    if args.check and not results[0][0]:
        print('\nThis endpoint fails the requested clearance threshold. '
              'A zero minimum also indicates an early exact intersection.')


if __name__ == '__main__':
    main()
