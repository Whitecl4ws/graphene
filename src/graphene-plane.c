/* graphene-plane.c: A plane in 3D space
 *
 * Copyright © 2014  Emmanuele Bassi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * SECTION:graphene-plane
 * @Title: Plane
 * @Short_Description: A plane in 3D space
 *
 * #graphene_plane_t is a structure representing a plane in 3D space, using
 * a normal vector pointing towards the origin, and a constant distance from
 * the origin along the normal vector.
 */

#include "graphene-private.h"

#include "graphene-plane.h"
#include "graphene-point3d.h"
#include "graphene-vec3.h"
#include "graphene-vec4.h"

#include <math.h>

/**
 * graphene_plane_alloc: (constructor)
 *
 * Allocates a new #graphene_plane_t structure.
 *
 * The contents of the returned structure are undefined.
 *
 * Returns: (transfer full): the newly allocated #graphene_plane_t.
 *   Use graphene_plane_free() to free the resources allocated by
 *   this function
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_alloc (void)
{
  return calloc (1, sizeof (graphene_plane_t));
}

/**
 * graphene_plane_free:
 * @p: a #graphene_plane_t
 *
 * Frees the resources allocated by graphene_plane_alloc().
 *
 * Since: 1.2
 */
void
graphene_plane_free (graphene_plane_t *p)
{
  free (p);
}

/**
 * graphene_plane_init:
 * @p: the #graphene_plane_t to initialize
 * @normal: (nullable): a normal vector defining the plane pointing towards the origin
 * @constant: the negative distance from the origin to the plane along the
 *   normal vector
 *
 * Initializes the given #graphene_plane_t using the given @normal vector
 * and @constant values.
 *
 * Returns: (transfer none): the initialized plane
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_init (graphene_plane_t      *p,
                     const graphene_vec3_t *normal,
                     float                  constant)
{
  if (normal != NULL)
    graphene_vec3_init_from_vec3 (&p->normal, normal);
  else
    graphene_vec3_init_from_vec3 (&p->normal, graphene_vec3_x_axis ());

  p->constant = constant;

  return p;
}

/**
 * graphene_plane_init_from_vec4:
 * @p: the #graphene_plane_t to initialize
 * @src: a #graphene_vec4_t containing the normal vector in its first
 *   three components, and the distance in its fourth component
 *
 * Initializes the given #graphene_plane_t using the components of
 * the given #graphene_vec4_t vector.
 *
 * Returns: (transfer none): the initialized plane
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_init_from_vec4 (graphene_plane_t      *p,
                               const graphene_vec4_t *src)
{
  graphene_vec4_get_xyz (src, &p->normal);
  p->constant = graphene_vec4_get_w (src);

  return p;
}

/**
 * graphene_plane_init_from_plane:
 * @p: the #graphene_plane_t to initialize
 * @src: a #graphene_plane_t
 *
 * Initializes the given #graphene_plane_t using the normal
 * vector and constant of another #graphene_plane_t.
 *
 * Returns: (transfer none): the initialized plane
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_init_from_plane (graphene_plane_t       *p,
                                const graphene_plane_t *src)
{
  graphene_vec3_init_from_vec3 (&p->normal, &src->normal);
  p->constant = src->constant;

  return p;
}

/**
 * graphene_plane_init_from_point:
 * @p: the #graphene_plane_t to initialize
 * @normal: a normal vector defining the plane pointing towards the origin
 * @point: a #graphene_point3d_t
 *
 * Initializes the given #graphene_plane_t using the given normal vector
 * and an arbitrary co-planar point.
 *
 * Returns: (transfer none): the initialized plane
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_init_from_point (graphene_plane_t         *p,
                                const graphene_vec3_t    *normal,
                                const graphene_point3d_t *point)
{
  graphene_vec3_t v_p;

  graphene_vec3_init_from_vec3 (&p->normal, normal);

  graphene_point3d_to_vec3 (point, &v_p);
  p->constant = graphene_vec3_dot (&v_p, &p->normal) * -1;

  return p;
}

/**
 * graphene_plane_init_from_points:
 * @p: the #graphene_plane_t to initialize
 * @a: a #graphene_point3d_t
 * @b: a #graphene_point3d_t
 * @c: a #graphene_point3d_t
 *
 * Initializes the given #graphene_plane_t using the 3 provided co-planar
 * points.
 *
 * The winding order is counter-clockwise, and determines which direction
 * the normal vector will point.
 *
 * Returns: (transfer none): the initialized plane
 *
 * Since: 1.2
 */
graphene_plane_t *
graphene_plane_init_from_points (graphene_plane_t         *p,
                                 const graphene_point3d_t *a,
                                 const graphene_point3d_t *b,
                                 const graphene_point3d_t *c)
{
  graphene_vec3_t v_a, v_b, v_c;
  graphene_vec3_t v1, v2;
  graphene_vec3_t normal;

  graphene_point3d_to_vec3 (a, &v_a);
  graphene_point3d_to_vec3 (b, &v_b);
  graphene_point3d_to_vec3 (c, &v_c);

  graphene_vec3_subtract (&v_c, &v_b, &v1);
  graphene_vec3_subtract (&v_a, &v_b, &v2);
  graphene_vec3_cross (&v1, &v2, &normal);
  graphene_vec3_normalize (&normal, &normal);

  return graphene_plane_init_from_point (p, &normal, a);
}

/**
 * graphene_plane_normalize:
 * @p: a #graphene_plane_t
 * @res: (out caller-allocates): return location for the normalized plane
 *
 * Normalizes the vector and constant of a #graphene_plane_t.
 *
 * Since: 1.2
 */
void
graphene_plane_normalize (const graphene_plane_t *p,
                          graphene_plane_t       *res)
{
  float normal_length = graphene_vec3_length (&p->normal);

  graphene_vec3_normalize (&p->normal, &res->normal);
  res->constant = p->constant / normal_length;
}

/**
 * graphene_plane_negate:
 * @p: a #graphene_plane_t
 * @res: (out caller-allocates): return location for the negated plane
 *
 * Negates the normal vector and constant of a #graphene_plane_t, effectively
 * mirroring the plane across the origin.
 *
 * Since: 1.2
 */
void
graphene_plane_negate (const graphene_plane_t *p,
                       graphene_plane_t       *res)
{
  graphene_vec3_negate (&p->normal, &res->normal);
  res->constant = p->constant * -1.f;
}

/**
 * graphene_plane_distance:
 * @p: a #graphene_plane_t
 * @point: a #graphene_point3d_t
 *
 * Computes the distance of @point from a #graphene_plane_t.
 *
 * Returns: the distance of the given #graphene_point3d_t from the plane
 *
 * Since: 1.2
 */
float
graphene_plane_distance (const graphene_plane_t   *p,
                         const graphene_point3d_t *point)
{
  graphene_vec3_t v;

  graphene_point3d_to_vec3 (point, &v);

  return graphene_vec3_dot (&p->normal, &v) + p->constant;
}

/**
 * graphene_plane_get_normal:
 * @p: a #graphene_plane_t
 * @normal: (out caller-allocates): return location for the normal vector
 *
 * Retrieves the normal vector pointing towards the origin of the
 * given #graphene_plane_t.
 *
 * Since: 1.2
 */
void
graphene_plane_get_normal (const graphene_plane_t *p,
                           graphene_vec3_t        *normal)
{
  graphene_vec3_init_from_vec3 (normal, &p->normal);
}

/**
 * graphene_plane_get_constant:
 * @p: a #graphene_plane_t
 *
 * Retrieves the distance along the normal vector of the
 * given #graphene_plane_t from the origin.
 *
 * Returns: the constant value of the plane
 *
 * Since: 1.2
 */
float
graphene_plane_get_constant (const graphene_plane_t *p)
{
  return p->constant;
}

/**
 * graphene_plane_equal:
 * @p1: a #graphene_plane_t
 * @p2: a #graphene_plane_t
 *
 * Checks whether the two given #graphene_plane_t are equal.
 *
 * Returns: `true` if the given planes are equal
 *
 * Since: 1.2
 */
bool
graphene_plane_equal (const graphene_plane_t *p1,
                      const graphene_plane_t *p2)
{
  if (p1 == p2)
    return true;

  if (p1 == NULL || p2 == NULL)
    return false;

  return graphene_vec3_equal (&p1->normal, &p2->normal) &&
         fabsf (p1->constant- p2->constant) < GRAPHENE_FLOAT_EPSILON;
}
