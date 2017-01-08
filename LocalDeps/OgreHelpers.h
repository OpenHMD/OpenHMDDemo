#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreRoot.h>

void setTransform(Ogre::Node* m_node, Ogre::Matrix4 mat4)
{
	if (m_node)
	{
		m_node->setPosition(mat4.getTrans());

		// extract rotation
		Ogre::Matrix3 matr;
		mat4.extract3x3Matrix(matr);
		matr.Orthonormalize();

		Ogre::Quaternion rot(matr);
		Ogre::Quaternion quat;
		if (quat.Norm() == 0.0)
			rot = Ogre::Quaternion::IDENTITY;
		quat = rot;

		m_node->setOrientation(quat);

		Ogre::Matrix4 invRot(rot.Inverse());
		Ogre::Matrix4 T = invRot * mat4;
		Ogre::Vector3 scale = Ogre::Vector3(T[0][0], T[1][1], T[2][2]);

		m_node->setScale(scale);
	}
}

void attachInPlace(Ogre::SceneNode* rootnode, Ogre::Node* nodeToParent, Ogre::Node* parentNode)
{
	Ogre::Matrix4 prevmat = nodeToParent->_getFullTransform();
	nodeToParent->getParent()->removeChild(nodeToParent);
	parentNode->addChild(nodeToParent);
	setTransform(nodeToParent, parentNode->_getFullTransform().inverse() * prevmat);
}