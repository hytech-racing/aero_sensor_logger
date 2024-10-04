{ stdenv, lib, cmake, protobuf, boost }:

stdenv.mkDerivation rec {
    pname = "aero_sensor_mcap_logger";
    version = "0.0.1";
    nativeBuildInputs = [ cmake ];
    propagatedBuildInputs = with lib; [ protobuf boost ];
}
