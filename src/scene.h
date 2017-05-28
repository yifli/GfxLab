#pragma once

#include "common.h"


class Scene {
public:
	void                            SetCamera(CameraPtr cam)      { _camera = cam; }
	void                            AddLight(LightPtr light)      { _lights.push_back(light); }
	void                            AddGeometry(GeometryPtr geom) { _geometries.push_back(geom); }
	const CameraPtr                 GetCamera()     const         { return _camera; }
	const std::vector<LightPtr>&    GetLights()     const         { return _lights; }
	const std::vector<GeometryPtr>& GetGeometries() const         { return _geometries; }

private:
	CameraPtr                _camera;
	std::vector<LightPtr>    _lights;
	std::vector<GeometryPtr> _geometries;
};