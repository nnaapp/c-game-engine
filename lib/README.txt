libimgui.a is a static library compiled with cImGui, Dear ImGui, and rlImGui together.
These are only used as a cohesive unit, so they are compiled into one lib for ease of use,
as well as minimizing the risk of errors. cImGui is provided with a makefile for a DLL, but 
static is just infinitely easier.
