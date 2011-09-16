#!/bin/sh

FORWARD_SHADING_OPT="-D_FORWARD_SHADING_ -D_NB_LIGHTS_=1"
DEFERRED_SHADING_OPT="-D_RENDER_TO_GBUFFER_ -D_NB_LIGHTS_=1"
DEPTH_PEELING_OPT="-D_DEPTH_PEELING_ -D_NB_LIGHTS_=1"

preproc ../general.frag --log=stdout $FORWARD_SHADING_OPT > general_forward.frag
preproc ../general.frag --log=stdout $DEFERRED_SHADING_OPT > general_deferred.frag
preproc ../general.frag --log=stdout $DEPTH_PEELING_OPT > general_depth_peeling.frag

preproc ../general.vert --log=stdout $FORWARD_SHADING_OPT > general_forward.vert
preproc ../general.vert --log=stdout $DEFERRED_SHADING_OPT > general_deferred.vert
preproc ../general.vert --log=stdout $DEPTH_PEELING_OPT > general_depth_peeling.vert
