with import <nixpkgs> {};

stdenv.mkDerivation {
    name = "SpikingC";
    src = ./.;

    installPhase = ''
        mkdir -p $out/bin
        cp SpikingC $out/bin
    '';
}