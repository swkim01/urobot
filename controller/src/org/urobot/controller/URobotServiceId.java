/*
 * Copyright (C) 2010 Teleal GmbH, Switzerland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.urobot.controller;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.teleal.cling.model.Constants;
import org.teleal.cling.model.types.InvalidValueException;
import org.teleal.cling.model.types.ServiceId;

/**
 * Service identifier with a fixed <code>upnp-org</code> namespace.
 * <p>
 * Also accepts the namespace sometimes used by broken devices, <code>schemas-upnp-org</code>.
 * </p>
 *
 * @author Christian Bauer
 */
public class URobotServiceId extends ServiceId {

    public static final String UROBOT_NAMESPACE = "urobot-org";
    public static final String BROKEN_UROBOT_NAMESPACE = "schemas-urobot-org";

    public static final Pattern PATTERN =
            Pattern.compile("urn:" + UROBOT_NAMESPACE + ":serviceId:(" + Constants.REGEX_ID+ ")");

    public static final Pattern BROKEN_PATTERN =
            Pattern.compile("urn:" + BROKEN_UROBOT_NAMESPACE + ":service:(" + Constants.REGEX_ID+ ")"); // Note: 'service' vs. 'serviceId'

    public URobotServiceId(String id) {
        super(UROBOT_NAMESPACE, id);
    }

    public static URobotServiceId valueOf(String s) throws InvalidValueException {
        Matcher matcher = URobotServiceId.PATTERN.matcher(s);
        if (matcher.matches()) {
            return new URobotServiceId(matcher.group(1));
        } else {
            matcher = URobotServiceId.BROKEN_PATTERN.matcher(s);
            if (matcher.matches()) {
                return new URobotServiceId(matcher.group(1));
            } else {
                throw new InvalidValueException("Can't parse UDA service ID string (upnp-org/id): " + s);
            }
        }
    }

}
