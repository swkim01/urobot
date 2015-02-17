package org.urobot.controller;

public interface JoystickMovedListener {
	public void OnMoved(float radial, float angle);
	public void OnReleased();
	public void OnReturnedToCenter();
}
