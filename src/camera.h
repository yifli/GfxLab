#pragma once
#include "common.h"


class Camera {
public:
	Camera(int x, int y, int viewWidth, int viewHeight);
	~Camera();

	void             LookAt(const glm::vec3& pos, const glm::vec3& focus, const glm::vec3& up);
	void             BeginRotate();
	void             Rotate(float x, float y);
	void             StopRotate();
	void             MoveForward();
	void             MoveBackward();
	void             MoveLeft();
	void             MoveRight();
	void             Zoom(float change);
	void             Resize(int width, int height);
	const glm::mat4& GetViewMatrix()                  const { return _view; }
	const glm::mat4& GetProjectionMatrix()            const { return _projection; }
	const glm::vec3& GetPosition()                    const { return _position; }



private:
	glm::vec3        MapToTrackball(float x, float y);
	glm::mat3        GetRotationMat(const glm::vec3& start, const glm::vec3& end);
	glm::vec3        FindVectorNotParallelTo(const glm::vec3& v);

	enum MovementType {
		NONE,
		ROTATE,
		ZOOM
	};

	static float  STEP_SIZE;

	int           _viewWidth, _viewHeight;
	int           _x, _y;
	float         _radius;
	float         _fovy;
	glm::vec3     _position;
	glm::vec3     _up;
	MovementType  _moveType;
	glm::mat4     _view;
	glm::mat4     _projection;
	glm::vec3     _lastTrackballPos;
	bool          _lastTrackballPosSet;
};