/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpoperationreplace.c
 * Copyright (C) 2008 Michael Natterer <mitch@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl-plugin.h>

#include "../operations-types.h"

#include "gimpoperationreplace.h"


static gboolean gimp_operation_replace_process (GeglOperation       *operation,
                                                void                *in_buf,
                                                void                *aux_buf,
                                                void                *aux2_buf,
                                                void                *out_buf,
                                                glong                samples,
                                                const GeglRectangle *roi,
                                                gint                 level);


G_DEFINE_TYPE (GimpOperationReplace, gimp_operation_replace,
               GIMP_TYPE_OPERATION_POINT_LAYER_MODE)


static void
gimp_operation_replace_class_init (GimpOperationReplaceClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gimp:replace",
                                 "description", "GIMP replace mode operation",
                                 NULL);

  point_class->process = gimp_operation_replace_process;
}

static void
gimp_operation_replace_init (GimpOperationReplace *self)
{
}

static gboolean
gimp_operation_replace_process (GeglOperation       *operation,
                                void                *in_buf,
                                void                *aux_buf,
                                void                *aux2_buf,
                                void                *out_buf,
                                glong                samples,
                                const GeglRectangle *roi,
                                gint                 level)
{
  GimpOperationPointLayerMode *layer_mode = (gpointer) operation;

  return gimp_operation_replace_process_pixels (in_buf, aux_buf, aux2_buf,
                                                out_buf,
                                                layer_mode->opacity,
                                                samples, roi, level,
                                                layer_mode->blend_trc,
                                                layer_mode->composite_trc,
                                                layer_mode->composite_mode);
}

gboolean
gimp_operation_replace_process_pixels (gfloat                *in,
                                       gfloat                *layer,
                                       gfloat                *mask,
                                       gfloat                *out,
                                       gfloat                 opacity,
                                       glong                  samples,
                                       const GeglRectangle   *roi,
                                       gint                   level,
                                       GimpLayerBlendTRC      blend_trc,
                                       GimpLayerBlendTRC      composite_trc,
                                       GimpLayerCompositeMode composite_mode)
{
  const gboolean has_mask = mask != NULL;

  while (samples--)
    {
      gfloat opacity_value = opacity;
      gfloat new_alpha;
      gint   b;

      if (has_mask)
        opacity_value *= *mask;

      new_alpha = (layer[ALPHA] - in[ALPHA]) * opacity_value + in[ALPHA];

      if (new_alpha)
        {
          gfloat ratio = opacity_value * layer[ALPHA] / new_alpha;

          for (b = RED; b < ALPHA; b++)
            out[b] = (layer[b] - in[b]) * ratio + in[b];
        }
      else
        {
          for (b = RED; b < ALPHA; b++)
            out[b] = in[b];
        }

      out[ALPHA] = new_alpha;

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}