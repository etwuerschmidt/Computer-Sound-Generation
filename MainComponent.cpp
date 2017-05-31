/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <algorithm>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public AudioAppComponent,
							   private ChangeListener,
							   private ButtonListener,
							   private Timer,
							   private ComboBox::Listener
{
public:
    //==============================================================================
    MainContentComponent()
		: state(Stopped),
		thumbnailCache(5),                            
		thumbnail(512, formatManager, thumbnailCache)
    {
		setLookAndFeel(&lookAndFeel);
		addAndMakeVisible(midiOutputListLabel);
		midiOutputListLabel.setText("MIDI Output:", dontSendNotification);
		midiOutputListLabel.attachToComponent(&midiOutputList, true);

		addAndMakeVisible(midiOutputList);
		midiOutputList.setTextWhenNoChoicesAvailable("No MIDI Output Enabled");
		midiOutputList.addItemList(MidiOutput::getDevices(), 1);
		midiOutputList.addListener(this);

		addAndMakeVisible(&openButton);
		openButton.setButtonText("Open...");
		openButton.addListener(this);

		addAndMakeVisible(&playButton);
		playButton.setButtonText("Play");
		playButton.addListener(this);
		playButton.setColour(TextButton::buttonColourId, Colours::green);
		playButton.setEnabled(false);

		addAndMakeVisible(&stopButton);
		stopButton.setButtonText("Stop");
		stopButton.addListener(this);
		stopButton.setColour(TextButton::buttonColourId, Colours::red);
		stopButton.setEnabled(false);

		setSize(600, 400);

		formatManager.registerBasicFormats();
		transportSource.addChangeListener(this);
		thumbnail.addChangeListener(this);            

		setAudioChannels(2, 2);
		startTimer(1);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
		transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
		monoBuffer = new float[samplesPerBlockExpected] {0};
		fs = sampleRate;
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
		if (readerSource == nullptr)
			bufferToFill.clearActiveBufferRegion();
		else
			transportSource.getNextAudioBlock(bufferToFill);

		AudioSampleBuffer* buffer;
		buffer = bufferToFill.buffer;
		const float* left = buffer->getReadPointer(0);
		const float* right = buffer->getReadPointer(1);
		const int numSamps = buffer->getNumSamples();

		for (int samp = 0; samp < numSamps; samp++)
		{
			monoBuffer[samp] = (left[samp] + right[samp]) / 2;
		}
    }

	void midiFFT(float* buffer, int size) {
		float bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8;
		bin1 = 0;
		bin2 = 0;
		bin3 = 0;
		bin4 = 0;
		bin5 = 0;
		bin6 = 0;
		bin7 = 0;
		bin8 = 0;
		MidiBuffer midiBuff;

		if (currentMidiOutput != nullptr) {
			int fftBuff = juce::nextPowerOfTwo(size); //512
			int points = log2(fftBuff);
			FFT fft(points - 1, 0); //8
			int fft_size = fft.getSize();
			fft.performFrequencyOnlyForwardTransform(buffer);

			Range<float> maxLevel = FloatVectorOperations::findMinAndMax(buffer, fftBuff / 2);
			float max_column = maxLevel.getEnd() * 64;
			int col_height1[8] = { 0, 16, 32, 48, 64, 80, 96, 112 };
			int col_height2[8] = { 1, 17, 33, 49, 65, 81, 97, 113 };
			int col_height3[8] = { 2, 18, 34, 50, 66, 82, 98, 114 };
			int col_height4[8] = { 3, 19, 35, 51, 67, 83, 99, 115 };
			int col_height5[8] = { 4, 20, 36, 52, 68, 84, 100, 116 };
			int col_height6[8] = { 5, 21, 37, 53, 69, 85, 101, 117 };
			int col_height7[8] = { 6, 22, 38, 54, 70, 86, 102, 118 };
			int col_height8[8] = { 7, 23, 39, 55, 71, 87, 103, 119 };

			for (int i = 0; i < 257; i++)
			{
				if (i >= 0 && i < 3) //0 - 172 Hz
					bin1 += buffer[i];
				else if (i >= 3 && i < 13) //258 - 1033 Hz
					bin2 += buffer[i];
				else if (i >= 13 && i < 36) //1119 - 3014 Hz
					bin3 += buffer[i];
				else if (i >= 36 && i < 70) //3100 - 5943 Hz
					bin4 += buffer[i];
				else if (i >= 70 && i < 117) //6029 - 9991 Hz
					bin5 += buffer[i];
				else if (i >= 117 && i < 163) //10077 - 13695 Hz
					bin6 += buffer[i];
				else if (i >= 163 && i < 198) //14039 - 16968 Hz
					bin7 += buffer[i];
				else if (i >= 198 && i < 209) //17054+ Hz --257
					bin8 += buffer[i];
			}

			float bins[] = { bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8 };
			float max_bin = *std::max_element(bins, bins + 8);
			if (max_bin != 0) {
				float block_size = max_bin / 8;
				int mod_bin1 = int(abs((bin1 / block_size) - 8));
				int mod_bin2 = int(abs((bin2 / block_size) - 8));
				int mod_bin3 = int(abs((bin3 / block_size) - 8));
				int mod_bin4 = int(abs((bin4 / block_size) - 8));
				int mod_bin5 = int(abs((bin5 / block_size) - 8));
				int mod_bin6 = int(abs((bin6 / block_size) - 8));
				int mod_bin7 = int(abs((bin7 / block_size) - 8));
				int mod_bin8 = int(abs((bin8 / block_size) - 8));
				for (int i = 0; i < 8; i++) {
					if (i < mod_bin1)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height1[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height1[i], 60));

					if (i < mod_bin2)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height2[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height2[i], 60));

					if (i < mod_bin3)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height3[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height3[i], 60));

					if (i < mod_bin4)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height4[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height4[i], 60));

					if (i < mod_bin5)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height5[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height5[i], 60));

					if (i < mod_bin6)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height6[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height6[i], 60));

					if (i < mod_bin7)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height7[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height7[i], 60));

					if (i < mod_bin8)
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height8[i], 0));
					else
						currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(144, col_height8[i], 60));
				}
			}
		}

	}

    void releaseResources() override
    {
		transportSource.releaseResources();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
		const Rectangle<int> thumbnailBounds(10, 130, getWidth() - 20, getHeight() - 120);

		if (thumbnail.getNumChannels() == 0)
			paintIfNoFileLoaded(g, thumbnailBounds);
		else
			paintIfFileLoaded(g, thumbnailBounds);
    }

    void resized() override
    {
		midiOutputList.setBounds(10, 10, getWidth() - 20, 20);
		openButton.setBounds(10, 40, getWidth() - 20, 20);
		playButton.setBounds(10, 70, getWidth() - 20, 20);
		stopButton.setBounds(10, 100, getWidth() - 20, 20);
    }

	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == &transportSource) transportSourceChanged();
		if (source == &thumbnail)       thumbnailChanged();
	}

	void buttonClicked(Button* button) override
	{
		if (button == &openButton)  openButtonClicked();
		if (button == &playButton)  playButtonClicked();
		if (button == &stopButton)  stopButtonClicked();
	}


private:
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	void timerCallback() override
	{
		midiFFT(monoBuffer, 480);
		repaint();
	}

	void changeState(TransportState newState)
	{
		if (state != newState)
		{
			state = newState;

			switch (state)
			{
			case Stopped:
				stopButton.setEnabled(false);
				playButton.setEnabled(true);
				transportSource.setPosition(0.0);
				currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(176, 0, 0));
				break;

			case Starting:
				playButton.setEnabled(false);
				transportSource.start();
				break;

			case Playing:
				stopButton.setEnabled(true);
				break;

			case Stopping:
				transportSource.stop();
				break;

			default:
				jassertfalse;
				break;
			}
		}
	}

	void comboBoxChanged(ComboBox* box) override
	{
		if (box == &midiOutputList)
			setMidiOutput(midiOutputList.getSelectedItemIndex());
	}

	void setMidiOutput(int index) {
		currentMidiOutput = nullptr;

		if (MidiOutput::getDevices()[index].isNotEmpty())
		{
			currentMidiOutput = MidiOutput::openDevice(index);
			jassert(currentMidiOutput);
			currentMidiOutput->sendMessageNow(MidiMessage::MidiMessage(176, 0, 0));
		}
	}

	void transportSourceChanged()
	{
		changeState(transportSource.isPlaying() ? Playing : Stopped);
	}

	void thumbnailChanged()
	{
		repaint();
	}

	void paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
	{
		g.setColour(Colours::darkgrey);
		g.fillRect(thumbnailBounds);
		g.setColour(Colours::white);
		g.drawFittedText("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);
	}

	void paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
	{
		g.setColour(Colours::white);
		g.fillRect(thumbnailBounds);

		g.setColour(Colours::red);                                     

		const double audioLength(thumbnail.getTotalLength());
		thumbnail.drawChannels(g,                                      
			thumbnailBounds,
			0.0,                                    // start time
			audioLength,             // end time
			1.0f);                                  // vertical zoom

		g.setColour(Colours::blue);
		const double audioPosition(transportSource.getCurrentPosition());
		const float drawPosition((audioPosition / audioLength) * thumbnailBounds.getWidth() + thumbnailBounds.getX());
		g.drawLine(drawPosition, thumbnailBounds.getY(), drawPosition, thumbnailBounds.getBottom(), 2.0f);


	}

	void openButtonClicked()
	{
		FileChooser chooser("Select a Wave file to play...",
			File::nonexistent,
			"*.wav");

		if (chooser.browseForFileToOpen())
		{
			File file(chooser.getResult());
			AudioFormatReader* reader = formatManager.createReaderFor(file);

			if (reader != nullptr)
			{
				ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
				transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
				playButton.setEnabled(true);
				thumbnail.setSource(new FileInputSource(file));          
				readerSource = newSource.release();
			}
		}
	}

	void playButtonClicked()
	{
		changeState(Starting);
	}

	void stopButtonClicked()
	{
		changeState(Stopping);
	}

	//==========================================================================
	TextButton openButton;
	TextButton playButton;
	TextButton stopButton;
	float* monoBuffer;
	Label midiOutputListLabel;
	ComboBox midiOutputList;
	ScopedPointer<MidiOutput> currentMidiOutput;
	double fs;


	AudioFormatManager formatManager;                    
	ScopedPointer<AudioFormatReaderSource> readerSource;
	AudioTransportSource transportSource;
	TransportState state;
	AudioThumbnailCache thumbnailCache;                  
	AudioThumbnail thumbnail;                            

	LookAndFeel_V3 lookAndFeel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
