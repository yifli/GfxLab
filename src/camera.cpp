#include "camera.h"
#include "mesh.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>


Camera::Camera(int x, int y, int viewWidth, int viewHeight)
	: _viewWidth(viewWidth),
	_viewHeight(viewHeight),
	_moveType(NONE),
	_lastTrackballPosSet(false)
{
	_fovy = glm::pi<float>() / 4.0f;
	_projection = glm::perspective(_fovy, float(_viewWidth) / float(_viewHeight), .1f, 1000.f);
}

Camera::~Camera()
{

}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& focus, const glm::vec3& up)
{
	_position = pos;
	_up = up;
	_view = glm::lookAt(pos, focus, up);
	_radius = glm::length(pos - focus);
}


void Camera::BeginMove()
{
	_moveType = ROTATE;
}

void Camera::StopMove()
{
	_moveType = NONE;
	_lastTrackballPosSet = false;
}

void Camera::Move(float x, float y)
{
	if (_moveType == ROTATE) {
		glm::vec3 curTrackballPos = MapToTrackball(x, y);
		if (_lastTrackballPosSet) {
			glm::mat3 rot = GetRotationMat(_lastTrackballPos, curTrackballPos);
			_position = rot * _position;
			_position = glm::normalize(_position) * _radius;
			_up = rot * _up;
			_view = glm::lookAt(_position, glm::vec3(0, 0, 0), _up);
		}
		else
			_lastTrackballPosSet = true;
		_lastTrackballPos = curTrackballPos;
	}
}

void Camera::Zoom(float change)
{
	float pi = glm::pi<float>();
	if (_fovy >= 0.3f && _fovy <= pi)
		_fovy += -change/10.0f;
	if (_fovy < 0.3f)
		_fovy = 0.3f;
	if (_fovy >= pi)
		_fovy = pi;
	_projection = glm::perspective(_fovy, float(_viewWidth) / float(_viewHeight), .1f, 1000.f);
}

void Camera::Resize(int width, int height)
{ 
	_viewWidth = width;
	_viewHeight = height;
	_projection = glm::perspective(_fovy, float(_viewWidth) / float(_viewHeight), .1f, 1000.f);
}



glm::vec3 Camera::MapToTrackball(float x, float y)
{
	glm::vec3 pointOnTrackball;
	pointOnTrackball.x = (2.0f * x - _viewWidth) / float(_viewWidth);
	pointOnTrackball.y = (_viewHeight - 2.0f * y) / float(_viewHeight);
	pointOnTrackball.z = .0f;

	float len = glm::length(pointOnTrackball);
	if (len > 1.0f)
		len = 1.0f;
	if (len * len > 1.0f)
		pointOnTrackball.z = .0f;
	else
		pointOnTrackball.z = sqrt(1.0f - len*len);
	pointOnTrackball = glm::normalize(pointOnTrackball);
	return pointOnTrackball;
}

glm::mat3 Camera::GetRotationMat(const glm::vec3& start, const glm::vec3& end)
{
	glm::vec3 s = glm::normalize(start);
	glm::vec3 t = glm::normalize(end);
	glm::vec3 u = glm::cross(s, t);
	float e = glm::dot(s, t);

	
	if (glm::length(u) <= 1e-3) { // s and t are parallel or near parallel
		if (fabs(1.0 - e) <= 1e-3) {
			// return identity matrix since rotation angle is close to zero
			return glm::mat3(1.0);
		}
		else {	// rotate pi radians
			t = FindVectorNotParallelTo(s);
			u = glm::cross(s, t);
			glm::quat rot(0, u);
			return glm::mat3(rot);
		}
	}
	
	float cos_half_angle = sqrtf(2 * (1 + e));
	u *= 1.0 / cos_half_angle;

	glm::quat rot(cos_half_angle * 0.5f, u);
	return glm::mat3(rot);
}

glm::vec3 Camera::FindVectorNotParallelTo(const glm::vec3& v)
{
	float vx, vy, vz;
	vx = v.x;
	vy = v.y;
	vz = v.z;

	glm::vec3 u;
	if (fabs(vx) < fabs(vy) && fabs(vx) < fabs(vz))
		u = glm::vec3(0, -vz, vy);
	else if (fabs(vy) < fabs(vx) && fabs(vy) < fabs(vz))
		u = glm::vec3(-vz, 0, vx);
	else
		u = glm::vec3(-vy, vx, 0);

	return glm::normalize(u);
}
