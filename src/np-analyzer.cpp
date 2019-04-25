
#include "np-analyzer.h"

void np::Analyzer::setup( int id, int sampleRate, int frames ) {
    
    engine.listDevices();
    
    engine.setChannels(2, 2);
    
    bands.resize(toInit.size());
    
    parameters.setName("analyzer");
    
    for( size_t i=0; i<bands.size(); ++i ){ 
		bands[i].setup( engine, toInit[i].name, toInit[i].channel );
		parameters.add( bands[i].ui );	
	}
   
    engine.setDeviceID(id);
    engine.setup( sampleRate, frames, 3); 
}


void np::Analyzer::defaults() {
    band( "band", 0 );
    setup( 0, 44100, 512 );
}


void np::Analyzer::band( std::string name, int channel ){
    toInit.emplace_back();
    toInit.back().name = name;
    toInit.back().channel = channel;
}

float np::Analyzer::meter( int band ) const {
    return bands[ band ].meter();
}

void np::Analyzer::draw( int x, int y ) {
    for( size_t i=0; i<bands.size(); ++i ){ 
		int yy = y +10 - 40*(bands.size()-i) ;
		ofDrawBitmapString( bands[i].ui.getName(), x, yy-7 );
		ofDrawRectangle( x, yy, meter(i)*200, 20 );
	}
}

float np::Analyzer::AudioAnalyzerBand::meter() const { 
    return envFollowerPeak.meter_output(); 
}


void np::Analyzer::AudioAnalyzerBand::setup( ofxPDSPEngine & engine, string name, int input ){
    
    rms.set( 50.0f );
    
    //engine.audio_in(input) >> bandGain >> loCut >> hiCut >> absolute >> envFollowerPeak >> engine.blackhole();
    engine.audio_in(input) >> loCut >> hiCut >> rms >> bandGain >> envFollowerPeak >> engine.blackhole();
    
    gainControl >> bandFader >> bandGain.in_mod();    

    // setting up controls and analyzerGui ---------------------
    loPitchControl >> p2fLo >> hiCut.in_freq();              
    hiPitchControl >> p2fHi >> loCut.in_freq();              
    
    peakAttackControl  >> envFollowerPeak.in_attack();
    peakReleaseControl >> envFollowerPeak.in_release();
        
    ui.setName( name );
    ui.add( gainControl.set( "band input gain", 0, -48, 24 ));  
    ui.add( loPitchControl.set( "band low", 43.0f, 20.0f, 136.0f ));  
    ui.add( loFreqValue.set(    "band lo hz", pdsp::p2f(43.0f), 25000.0f, 26000.0f ));  
    ui.add( hiPitchControl.set( "band hi", 100.0f, 20.0f, 136.0f ));  
    ui.add( hiFreqValue.set(    "band hi hz", pdsp::p2f(100.0f), 25000.0f, 26000.0f ));   
    ui.add( peakAttackControl.set( "env attack ms", 10, 1, 80 ));
    ui.add( peakReleaseControl.set( "env release ms", 200, 1, 500 ));

    loPitchControl.getOFParameterFloat().addListener(this, &AudioAnalyzerBand::freqLoChanged);
    hiPitchControl.getOFParameterFloat().addListener(this, &AudioAnalyzerBand::freqHiChanged);
}

void np::Analyzer::AudioAnalyzerBand::freqLoChanged(float & pitch) {
    loFreqValue = pdsp::p2f(pitch);
}

void np::Analyzer::AudioAnalyzerBand::freqHiChanged(float & pitch) {
    hiFreqValue = pdsp::p2f(pitch);    
}

void np::Analyzer::setBandLowPitch( float value, int band ){
    bands[band].loPitchControl.getOFParameterFloat() = value;
}

void np::Analyzer::setBandHighPitch( float value, int band ){
    bands[band].hiPitchControl.getOFParameterFloat() = value;
}
    
