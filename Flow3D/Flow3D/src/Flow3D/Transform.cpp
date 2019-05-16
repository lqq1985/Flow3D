#include "Transform.hpp"

#include "Log.hpp"

namespace Flow {

	Transform::Transform(const Vec3& position, const Vec3& rotation, const Vec3& scale)
		: m_Position(position), m_Rotation(rotation), m_Scale(scale)
	{
		m_WorldUp = Vec3(0.0f, 1.0f, 0.0f);
		m_RotationQuaternion = Quaternion(m_Rotation);

		UpdateVectors();
	}

	Transform::~Transform()
	{
	}

	void Transform::Translate(const Vec3& translation)
	{
		m_Position += translation;
	}

	void Transform::Rotate(const Vec3 & axis, float angle)
	{
		Rotate(Quaternion(axis, angle));
	}

	void Transform::Rotate(const Quaternion& rotation)
	{
		m_RotationQuaternion = Quaternion(m_RotationQuaternion * rotation).Normalize();
		m_Rotation = m_RotationQuaternion.ToEulerAngles();
		UpdateVectors();
	}

	void Transform::SetRotation(const Vec3& newRotation)
	{
		m_Rotation.z = newRotation.z;
		m_Rotation.y = newRotation.y;
		m_RotationQuaternion = Quaternion(newRotation);
		UpdateVectors();
	}

	Mat4 Transform::GetTransformation() const
	{
		Mat4 translationMatrix = Mat4();
		translationMatrix.Translate(m_Position);

		Mat4 scaleMatrix = Mat4();
		scaleMatrix.Scale(m_Scale);

		Mat4 rotationMatrix = m_RotationQuaternion.ToRotationMatrix();

		Mat4 result = translationMatrix * rotationMatrix * scaleMatrix;
		if (m_Parent != nullptr)
			result = m_Parent->GetTransformation() * result;

		return result;
	}

	const Vec3 Transform::GetPosition() const
	{
		Vec3 position = Vec3(0.0f);
		if (m_Parent != nullptr)
			position = m_Parent->GetPosition();

		position += m_Position;

		return position;
	}

	const Vec3 Transform::GetRotation() const
	{
		Vec3 rotation = Vec3(0.0f);
		if (m_Parent != nullptr)
			rotation = m_Parent->GetRotation();

		rotation += m_Rotation;

		return rotation;
	}

	const Vec3 Transform::GetScale() const
	{
		Vec3 scale = Vec3(1.0f);
		if (m_Parent != nullptr)
		{
			Vec3 parentScale = m_Parent->GetScale();
			scale.x *= parentScale.x;
			scale.y *= parentScale.y;
			scale.z *= parentScale.z;
		}
		
		scale.x *= m_Scale.x;
		scale.y *= m_Scale.y;
		scale.z *= m_Scale.z;

		return scale;
	}

	void Transform::UpdateVectors()
	{
		Vec3 front;
		front.x = Math::Cos(m_Rotation.z) * Math::Cos(m_Rotation.y);
		front.y = Math::Sin(m_Rotation.y);
		front.z = Math::Sin(m_Rotation.z) * Math::Cos(m_Rotation.y);
		m_Front = front.Normalize();
		m_Right = Vec3::Cross(m_Front, m_WorldUp).Normalize();
		m_Up = Vec3::Cross(m_Right, m_Front).Normalize();
	}
}