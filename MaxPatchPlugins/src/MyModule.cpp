#include "plugin.hpp"
#include <random> // Include for randomness

struct MyModule : Module {
    enum ParamId {
        SPEED_KNOB,
        PULSE_WIDTH_KNOB,
        GATE_KNOB,
        BLEED_KNOB,
        PARAMS_LEN
    };
    enum InputId {
        SPEED_INPUT,
        PULSE_WIDTH_INPUT,
        GATE_INPUT,
        BLEED_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        SQUARE_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        BLINK_LIGHT,
        LIGHTS_LEN
    };

    float phase = 0.f;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    bool skipCycle = false;
    int bleedCounter = 0;

    MyModule() : gen(rd()), dis(0.f, 1.f) {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(SPEED_KNOB, 0.1f, 20.f, 1.f, "Speed", " Hz");
        configParam(PULSE_WIDTH_KNOB, 0.01f, 0.99f, 0.5f, "Pulse Width");
        configParam(GATE_KNOB, 0.f, 1.f, 0.f, "Gate");
        configParam(BLEED_KNOB, 0.f, 1.f, 0.f, "Bleed");
    }

    void process(const ProcessArgs& args) override {
        float frequency = params[SPEED_KNOB].getValue();
        float pulseWidth = params[PULSE_WIDTH_KNOB].getValue();
        float gate = params[GATE_KNOB].getValue();
        float bleed = params[BLEED_KNOB].getValue();

        phase += frequency * args.sampleTime;
        if (phase >= 1.f) {
            phase -= 1.f;

            if (bleedCounter > 0) {
                skipCycle = false;
                bleedCounter--;
            } else {
                skipCycle = dis(gen) < gate;
                if (!skipCycle && bleed > 0.f) {
                    bleedCounter = static_cast<int>(dis(gen) * bleed * 10);
                }
            }
        }

        float squareWave = (!skipCycle && phase < pulseWidth) ? 10.f : 0.f;
        outputs[SQUARE_OUTPUT].setVoltage(squareWave);
        lights[BLINK_LIGHT].setBrightness(!skipCycle && phase < pulseWidth ? 1.f : 0.f);
    }
};

struct MyModuleWidget : ModuleWidget {
    MyModuleWidget(MyModule* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MyModule.svg")));

        // Add screws
        addChild(createWidget<ScrewSilver>(Vec(15, 0))); // Top-left screw
        addChild(createWidget<ScrewSilver>(Vec(120, 0))); // Top-right screw
        addChild(createWidget<ScrewSilver>(Vec(15, 365))); // Bottom-left screw
        addChild(createWidget<ScrewSilver>(Vec(120, 365))); // Bottom-right screw

        // Speed knob and input
        addParam(createParamCentered<RoundBlackKnob>(Vec(30, 50), module, MyModule::SPEED_KNOB)); // Speed knob
        addInput(createInputCentered<PJ301MPort>(Vec(100, 50), module, MyModule::SPEED_INPUT));   // Speed input

        // Pulse Width knob and input
        addParam(createParamCentered<RoundBlackKnob>(Vec(30, 110), module, MyModule::PULSE_WIDTH_KNOB)); // Pulse Width knob
        addInput(createInputCentered<PJ301MPort>(Vec(100, 110), module, MyModule::PULSE_WIDTH_INPUT));   // Pulse Width input

        // Gate knob and input
        addParam(createParamCentered<RoundBlackKnob>(Vec(30, 170), module, MyModule::GATE_KNOB)); // Gate knob
        addInput(createInputCentered<PJ301MPort>(Vec(100, 170), module, MyModule::GATE_INPUT));   // Gate input

        // Bleed knob and input
        addParam(createParamCentered<RoundBlackKnob>(Vec(30, 230), module, MyModule::BLEED_KNOB)); // Bleed knob
        addInput(createInputCentered<PJ301MPort>(Vec(100, 230), module, MyModule::BLEED_INPUT));   // Bleed input

        // Output
        addOutput(createOutputCentered<PJ301MPort>(Vec(65, 300), module, MyModule::SQUARE_OUTPUT)); // Output port

        // LED
        addChild(createLightCentered<MediumLight<RedLight>>(Vec(65, 340), module, MyModule::BLINK_LIGHT)); // LED
    }
};

Model* modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");