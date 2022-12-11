package org.clangen.autom8.device;

public interface SecuritySensor extends Device {
	boolean isTripped();
	boolean isArmed();
}
