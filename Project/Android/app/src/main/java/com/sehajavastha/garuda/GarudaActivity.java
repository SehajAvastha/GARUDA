package com.sehajavastha.garuda;

import org.libsdl.app.SDLActivity;

public class GarudaActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "c++_shared",
            "main"
        };
    }
}