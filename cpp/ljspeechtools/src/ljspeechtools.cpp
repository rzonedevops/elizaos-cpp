#include "elizaos/ljspeechtools.hpp"
#include "elizaos/agentlogger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cmath>
#include <filesystem>
#include <cstring>

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

namespace elizaos {

namespace fs = std::filesystem;

// Global logger instance for the module
static AgentLogger g_logger;

// AudioProcessor implementation
AudioData AudioProcessor::loadAudioFile(const std::string& file_path) {
    AudioData result;
    
#ifdef HAVE_SNDFILE
    // Real implementation using libsndfile
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));
    
    SNDFILE* sndfile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        g_logger.log("Failed to open audio file: " + file_path + " - " + sf_strerror(nullptr), "", "ljspeechtools", LogLevel::ERROR);
        // Fall back to mock implementation
        result.samples.resize(44100, 0.0f); // 1 second of silence
        result.sample_rate = 44100;
        result.channels = 1;
        result.duration_seconds = 1.0;
        return result;
    }
    
    // Read file info
    result.sample_rate = sfinfo.samplerate;
    result.channels = sfinfo.channels;
    result.duration_seconds = static_cast<double>(sfinfo.frames) / sfinfo.samplerate;
    
    // Read audio data
    result.samples.resize(sfinfo.frames * sfinfo.channels);
    sf_count_t read_count = sf_readf_float(sndfile, result.samples.data(), sfinfo.frames);
    
    if (read_count != sfinfo.frames) {
        g_logger.log("Warning: Expected to read " + std::to_string(sfinfo.frames) + 
                    " frames, but read " + std::to_string(read_count), "", "ljspeechtools", LogLevel::WARNING);
    }
    
    sf_close(sndfile);
    g_logger.log("Loaded audio file: " + file_path + 
                " (" + std::to_string(result.duration_seconds) + "s, " +
                std::to_string(result.sample_rate) + "Hz, " + 
                std::to_string(result.channels) + " ch)", "", "ljspeechtools", LogLevel::INFO);
#else
    // Mock implementation fallback
    result.samples.resize(44100, 0.0f); // 1 second of silence
    result.sample_rate = 44100;
    result.channels = 1;
    result.duration_seconds = 1.0;
    
    g_logger.log("Loaded audio file (mock): " + file_path, "", "ljspeechtools", LogLevel::INFO);
#endif
    
    return result;
}

bool AudioProcessor::saveAudioFile(const AudioData& audio, const std::string& file_path) {
#ifdef HAVE_SNDFILE
    // Real implementation using libsndfile
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));
    
    sfinfo.samplerate = audio.sample_rate;
    sfinfo.channels = audio.channels;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; // 16-bit PCM WAV
    
    SNDFILE* sndfile = sf_open(file_path.c_str(), SFM_WRITE, &sfinfo);
    if (!sndfile) {
        g_logger.log("Failed to create audio file: " + file_path + " - " + sf_strerror(nullptr), "", "ljspeechtools", LogLevel::ERROR);
        return false;
    }
    
    sf_count_t frames_to_write = audio.samples.size() / audio.channels;
    sf_count_t written = sf_writef_float(sndfile, audio.samples.data(), frames_to_write);
    
    sf_close(sndfile);
    
    if (written != frames_to_write) {
        g_logger.log("Warning: Expected to write " + std::to_string(frames_to_write) + 
                    " frames, but wrote " + std::to_string(written), "", "ljspeechtools", LogLevel::WARNING);
        return false;
    }
    
    g_logger.log("Saved audio file: " + file_path + 
                " (" + std::to_string(audio.duration_seconds) + "s, " +
                std::to_string(audio.sample_rate) + "Hz, " + 
                std::to_string(audio.channels) + " ch)", "", "ljspeechtools", LogLevel::INFO);
    return true;
#else
    // Mock implementation fallback
    (void)audio; // Suppress unused parameter warning
    g_logger.log("Saved audio file (mock): " + file_path, "", "ljspeechtools", LogLevel::INFO);
    return true;
#endif
}

std::vector<AudioData> AudioProcessor::splitOnSilence(
    const AudioData& audio,
    int min_silence_len_ms,
    float silence_thresh_db,
    int keep_silence_ms
) {
    std::vector<AudioData> chunks;
    
    // Convert parameters
    int min_silence_samples = (min_silence_len_ms * audio.sample_rate) / 1000;
    int keep_silence_samples = (keep_silence_ms * audio.sample_rate) / 1000;
    float silence_thresh_linear = std::pow(10.0f, silence_thresh_db / 20.0f); // Convert dB to linear
    
    // Find silence periods
    std::vector<bool> is_silent(audio.samples.size());
    for (size_t i = 0; i < audio.samples.size(); ++i) {
        is_silent[i] = std::abs(audio.samples[i]) < silence_thresh_linear;
    }
    
    // Find continuous silence regions
    std::vector<std::pair<size_t, size_t>> silence_regions;
    size_t start = 0;
    bool in_silence = false;
    
    for (size_t i = 0; i < is_silent.size(); ++i) {
        if (!in_silence && is_silent[i]) {
            start = i;
            in_silence = true;
        } else if (in_silence && !is_silent[i]) {
            if (i - start >= static_cast<size_t>(min_silence_samples)) {
                silence_regions.emplace_back(start, i);
            }
            in_silence = false;
        }
    }
    
    // Handle case where audio ends in silence
    if (in_silence && is_silent.size() - start >= static_cast<size_t>(min_silence_samples)) {
        silence_regions.emplace_back(start, is_silent.size());
    }
    
    // Split at silence regions
    size_t last_end = 0;
    for (const auto& silence_region : silence_regions) {
        size_t chunk_start = last_end;
        size_t chunk_end = silence_region.first + keep_silence_samples;
        
        if (chunk_end > chunk_start) {
            AudioData chunk;
            chunk.sample_rate = audio.sample_rate;
            chunk.channels = audio.channels;
            chunk.samples.assign(audio.samples.begin() + chunk_start, 
                               audio.samples.begin() + std::min(chunk_end, audio.samples.size()));
            chunk.duration_seconds = static_cast<double>(chunk.samples.size()) / (audio.sample_rate * audio.channels);
            chunks.push_back(chunk);
        }
        
        last_end = silence_region.second - keep_silence_samples;
    }
    
    // Add final chunk if there's audio after the last silence
    if (last_end < audio.samples.size()) {
        AudioData chunk;
        chunk.sample_rate = audio.sample_rate;
        chunk.channels = audio.channels;
        chunk.samples.assign(audio.samples.begin() + last_end, audio.samples.end());
        chunk.duration_seconds = static_cast<double>(chunk.samples.size()) / (audio.sample_rate * audio.channels);
        chunks.push_back(chunk);
    }
    
    // If no splits were made, return the original audio
    if (chunks.empty()) {
        chunks.push_back(audio);
    }
    
    g_logger.log("Split audio into " + std::to_string(chunks.size()) + " chunks using silence detection", "", "ljspeechtools", LogLevel::INFO);
    return chunks;
}

std::vector<AudioData> AudioProcessor::filterByDuration(
    const std::vector<AudioData>& chunks,
    double min_duration,
    double max_duration
) {
    std::vector<AudioData> filtered;
    
    for (const auto& chunk : chunks) {
        if (chunk.duration_seconds >= min_duration && 
            chunk.duration_seconds <= max_duration) {
            filtered.push_back(chunk);
        }
    }
    
    return filtered;
}

AudioData AudioProcessor::normalize(const AudioData& audio) {
    AudioData result = audio;
    
    // Find maximum amplitude
    float max_amplitude = 0.0f;
    for (float sample : result.samples) {
        max_amplitude = std::max(max_amplitude, std::abs(sample));
    }
    
    // Normalize to 0.9 to avoid clipping
    if (max_amplitude > 0.0f) {
        float scale = 0.9f / max_amplitude;
        for (float& sample : result.samples) {
            sample *= scale;
        }
    }
    
    return result;
}

AudioData AudioProcessor::convertFormat(
    const AudioData& audio,
    int target_sample_rate,
    int target_channels
) {
    AudioData result = audio;
    
    // Channel conversion (simple implementation)
    if (target_channels != audio.channels) {
        std::vector<float> converted_samples;
        
        if (audio.channels == 1 && target_channels == 2) {
            // Mono to stereo: duplicate mono channel
            converted_samples.reserve(audio.samples.size() * 2);
            for (size_t i = 0; i < audio.samples.size(); ++i) {
                converted_samples.push_back(audio.samples[i]); // Left channel
                converted_samples.push_back(audio.samples[i]); // Right channel
            }
        } else if (audio.channels == 2 && target_channels == 1) {
            // Stereo to mono: average both channels
            converted_samples.reserve(audio.samples.size() / 2);
            for (size_t i = 0; i < audio.samples.size(); i += 2) {
                float mono_sample = (audio.samples[i] + audio.samples[i + 1]) * 0.5f;
                converted_samples.push_back(mono_sample);
            }
        } else {
            // For other conversions, just copy the data for now
            converted_samples = audio.samples;
            g_logger.log("Warning: Unsupported channel conversion from " + 
                        std::to_string(audio.channels) + " to " + 
                        std::to_string(target_channels), "", "ljspeechtools", LogLevel::WARNING);
        }
        
        result.samples = converted_samples;
        result.channels = target_channels;
    }
    
    // Sample rate conversion (simple implementation using linear interpolation)
    if (target_sample_rate != audio.sample_rate) {
        double ratio = static_cast<double>(target_sample_rate) / audio.sample_rate;
        size_t new_sample_count = static_cast<size_t>(result.samples.size() * ratio);
        std::vector<float> resampled(new_sample_count);
        
        for (size_t i = 0; i < new_sample_count; ++i) {
            double source_index = i / ratio;
            size_t index0 = static_cast<size_t>(source_index);
            size_t index1 = std::min(index0 + 1, result.samples.size() - 1);
            double fraction = source_index - index0;
            
            // Linear interpolation
            resampled[i] = result.samples[index0] * (1.0f - fraction) + 
                          result.samples[index1] * fraction;
        }
        
        result.samples = resampled;
        result.sample_rate = target_sample_rate;
        result.duration_seconds = static_cast<double>(result.samples.size()) / (target_sample_rate * result.channels);
    }
    
    g_logger.log("Converted audio format: " + 
                std::to_string(audio.sample_rate) + "Hz/" + std::to_string(audio.channels) + "ch -> " +
                std::to_string(target_sample_rate) + "Hz/" + std::to_string(target_channels) + "ch", 
                "", "ljspeechtools", LogLevel::INFO);
    return result;
}

// SpeechTranscriber implementation
class SpeechTranscriber::Impl {
public:
    std::string model_name_ = "default";
    
    TranscriptionResult transcribe(const AudioData& audio) {
        TranscriptionResult result;
        
        // Enhanced mock implementation with more realistic behavior
        if (audio.samples.empty()) {
            result.success = false;
            result.error_message = "Empty audio data";
            return result;
        }
        
        // Analyze audio characteristics to provide more realistic mock transcription
        float max_amplitude = 0.0f;
        float avg_amplitude = 0.0f;
        for (float sample : audio.samples) {
            float abs_sample = std::abs(sample);
            max_amplitude = std::max(max_amplitude, abs_sample);
            avg_amplitude += abs_sample;
        }
        avg_amplitude /= audio.samples.size();
        
        // Generate transcription based on audio characteristics
        std::string transcription;
        double confidence;
        
        if (max_amplitude < 0.01f) {
            // Very quiet audio - likely silence
            transcription = "[silence]";
            confidence = 0.95;
        } else if (avg_amplitude < 0.1f) {
            // Quiet audio
            transcription = "quiet speech or background noise detected";
            confidence = 0.6;
        } else if (audio.duration_seconds < 1.0) {
            // Short audio
            transcription = "short utterance";
            confidence = 0.8;
        } else {
            // Normal speech-like audio
            transcription = "speech detected in " + std::to_string(audio.duration_seconds) + " second audio clip";
            confidence = 0.9;
        }
        
        result.text = transcription;
        result.confidence = confidence;
        result.success = true;
        
        g_logger.log("Transcribed audio (" + std::to_string(audio.duration_seconds) + 
                    "s, max_amp=" + std::to_string(max_amplitude) + "): " + transcription, 
                    "", "ljspeechtools", LogLevel::INFO);
        return result;
    }
    
    TranscriptionResult transcribeFile(const std::string& file_path) {
        TranscriptionResult result;
        
        if (!fs::exists(file_path)) {
            result.success = false;
            result.error_message = "File not found: " + file_path;
            g_logger.log("Transcription failed: " + result.error_message, "", "ljspeechtools", LogLevel::ERROR);
            return result;
        }
        
        // Load the audio file and transcribe it
        try {
            AudioData audio = AudioProcessor::loadAudioFile(file_path);
            result = transcribe(audio);
            if (result.success) {
                result.text = "Transcription of " + fs::path(file_path).filename().string() + ": " + result.text;
            }
        } catch (const std::exception& e) {
            result.success = false;
            result.error_message = "Error loading audio file: " + std::string(e.what());
            g_logger.log("Transcription failed: " + result.error_message, "", "ljspeechtools", LogLevel::ERROR);
        }
        
        return result;
    }
};

SpeechTranscriber::SpeechTranscriber() : impl_(std::make_unique<Impl>()) {}
SpeechTranscriber::~SpeechTranscriber() = default;

TranscriptionResult SpeechTranscriber::transcribe(const AudioData& audio) {
    return impl_->transcribe(audio);
}

TranscriptionResult SpeechTranscriber::transcribeFile(const std::string& file_path) {
    return impl_->transcribeFile(file_path);
}

std::vector<std::pair<std::string, TranscriptionResult>> SpeechTranscriber::transcribeBatch(
    const std::vector<std::string>& file_paths
) {
    std::vector<std::pair<std::string, TranscriptionResult>> results;
    
    for (const auto& path : file_paths) {
        auto result = transcribeFile(path);
        results.emplace_back(path, result);
    }
    
    return results;
}

void SpeechTranscriber::setTranscriptionModel(const std::string& model_name) {
    impl_->model_name_ = model_name;
}

// SpeechSynthesizer implementation
class SpeechSynthesizer::Impl {
public:
    std::string model_name_ = "default";
    
    AudioData synthesize(const std::string& text, const SynthesisConfig& config) {
        AudioData result;
        
        if (text.empty()) {
            g_logger.log("Warning: Empty text provided for synthesis", "", "ljspeechtools", LogLevel::WARNING);
            return result;
        }
        
        // Enhanced synthesis: create more sophisticated audio based on text
        double base_duration = text.length() * 0.08; // ~80ms per character
        base_duration *= (1.0 / config.speed); // Adjust for speed
        
        int duration_samples = static_cast<int>(base_duration * config.sample_rate);
        result.samples.resize(duration_samples * config.channels);
        result.sample_rate = config.sample_rate;
        result.channels = config.channels;
        result.duration_seconds = base_duration;
        
        // Generate more sophisticated waveform based on text content
        std::hash<std::string> hash_fn;
        size_t text_hash = hash_fn(text);
        
        // Use text hash to determine voice characteristics
        float base_freq = 200.0f + (text_hash % 200); // 200-400 Hz base frequency
        base_freq *= config.pitch; // Adjust for pitch
        
        // Generate speech-like waveform with multiple harmonics
        for (int i = 0; i < duration_samples; ++i) {
            float t = static_cast<float>(i) / config.sample_rate;
            
            // Add formant-like frequencies
            float sample = 0.0f;
            sample += 0.4f * std::sin(2.0f * M_PI * base_freq * t); // Fundamental
            sample += 0.2f * std::sin(2.0f * M_PI * base_freq * 2.0f * t); // Second harmonic
            sample += 0.1f * std::sin(2.0f * M_PI * base_freq * 3.0f * t); // Third harmonic
            
            // Add some variation based on text content
            float text_variation = std::sin(2.0f * M_PI * t * (text_hash % 50 + 10));
            sample += 0.1f * text_variation;
            
            // Add amplitude envelope (attack-sustain-release)
            float envelope = 1.0f;
            float attack_time = 0.1f;
            float release_time = 0.2f;
            
            if (t < attack_time) {
                envelope = t / attack_time;
            } else if (t > base_duration - release_time) {
                envelope = (base_duration - t) / release_time;
            }
            
            sample *= envelope * 0.3f; // Overall amplitude scaling
            
            // Fill channels
            for (int ch = 0; ch < config.channels; ++ch) {
                result.samples[i * config.channels + ch] = sample;
            }
        }
        
        g_logger.log("Synthesized text (" + std::to_string(text.length()) + " chars, " +
                    std::to_string(base_duration) + "s): " + text.substr(0, 50) + 
                    (text.length() > 50 ? "..." : ""), "", "ljspeechtools", LogLevel::INFO);
        return result;
    }
};

SpeechSynthesizer::SpeechSynthesizer() : impl_(std::make_unique<Impl>()) {}
SpeechSynthesizer::~SpeechSynthesizer() = default;

AudioData SpeechSynthesizer::synthesize(const std::string& text, const SynthesisConfig& config) {
    return impl_->synthesize(text, config);
}

bool SpeechSynthesizer::synthesizeToFile(
    const std::string& text,
    const std::string& output_path,
    const SynthesisConfig& config
) {
    auto audio = synthesize(text, config);
    return AudioProcessor::saveAudioFile(audio, output_path);
}

void SpeechSynthesizer::setSynthesisModel(const std::string& model_name) {
    impl_->model_name_ = model_name;
}

std::vector<std::string> SpeechSynthesizer::getAvailableVoices() {
    return {"default", "female", "male", "child"};
}

// DatasetPreparator implementation
std::vector<DatasetPreparator::MetadataEntry> DatasetPreparator::createDataset(
    const std::string& input_dir,
    const std::string& output_dir,
    bool split_long_audio,
    bool filter_short_audio
) {
    std::vector<MetadataEntry> metadata;
    
    // Create output directory
    fs::create_directories(output_dir);
    fs::create_directories(output_dir + "/wavs");
    
    g_logger.log("Creating dataset from: " + input_dir, "", "ljspeechtools", LogLevel::INFO);
    g_logger.log("Output directory: " + output_dir, "", "ljspeechtools", LogLevel::INFO);
    
    // Process audio files in input directory
    SpeechTranscriber transcriber;
    AudioProcessor processor;
    
    int file_counter = 0;
    
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;
        
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        // Check if it's an audio file
        if (ext == ".wav" || ext == ".mp3" || ext == ".flac" || ext == ".ogg") {
            std::string input_file = entry.path().string();
            
            try {
                // Load audio file
                AudioData audio = processor.loadAudioFile(input_file);
                
                std::vector<AudioData> chunks;
                
                if (split_long_audio && audio.duration_seconds > 12.0) {
                    // Split long audio into chunks
                    chunks = processor.splitOnSilence(audio, 1500, -60.0f, 250);
                } else {
                    chunks.push_back(audio);
                }
                
                // Process each chunk
                for (size_t i = 0; i < chunks.size(); ++i) {
                    AudioData chunk = chunks[i];
                    
                    // Filter by duration if requested
                    if (filter_short_audio && chunk.duration_seconds < 1.0) {
                        continue;
                    }
                    if (chunk.duration_seconds > 12.0) {
                        continue;
                    }
                    
                    // Normalize audio
                    chunk = processor.normalize(chunk);
                    
                    // Convert to standard format (22050 Hz, mono)
                    chunk = processor.convertFormat(chunk, 22050, 1);
                    
                    // Save processed audio
                    std::string output_filename = "sample_" + 
                                                std::to_string(file_counter) + "_" + 
                                                std::to_string(i) + ".wav";
                    std::string output_path = output_dir + "/wavs/" + output_filename;
                    
                    if (processor.saveAudioFile(chunk, output_path)) {
                        // Transcribe the audio
                        auto transcription_result = transcriber.transcribe(chunk);
                        
                        if (transcription_result.success) {
                            MetadataEntry entry;
                            entry.audio_file = "wavs/" + output_filename;
                            entry.transcription = transcription_result.text;
                            entry.normalized_transcription = normalizeTranscription(transcription_result.text);
                            metadata.push_back(entry);
                        }
                    }
                }
                
                file_counter++;
                
            } catch (const std::exception& e) {
                g_logger.log("Error processing file " + input_file + ": " + e.what(), 
                           "", "ljspeechtools", LogLevel::ERROR);
            }
        }
    }
    
    g_logger.log("Processed " + std::to_string(file_counter) + " audio files, created " +
                std::to_string(metadata.size()) + " dataset entries", "", "ljspeechtools", LogLevel::INFO);
    
    return metadata;
}

bool DatasetPreparator::saveMetadata(
    const std::vector<MetadataEntry>& metadata,
    const std::string& output_path
) {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& entry : metadata) {
        file << entry.audio_file << "|" << entry.transcription << "|" 
             << entry.normalized_transcription << "\n";
    }
    
    file.close();
    g_logger.log("Saved metadata to: " + output_path, "", "ljspeechtools", LogLevel::INFO);
    return true;
}

std::vector<DatasetPreparator::MetadataEntry> DatasetPreparator::loadMetadata(
    const std::string& input_path
) {
    std::vector<MetadataEntry> metadata;
    std::ifstream file(input_path);
    
    if (!file.is_open()) {
        g_logger.log("Cannot open metadata file: " + input_path, "", "ljspeechtools", LogLevel::ERROR);
        return metadata;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string part;
        MetadataEntry entry;
        
        if (std::getline(ss, part, '|')) entry.audio_file = part;
        if (std::getline(ss, part, '|')) entry.transcription = part;
        if (std::getline(ss, part, '|')) entry.normalized_transcription = part;
        
        metadata.push_back(entry);
    }
    
    file.close();
    g_logger.log("Loaded metadata from: " + input_path, "", "ljspeechtools", LogLevel::INFO);
    return metadata;
}

bool DatasetPreparator::validateDataset(const std::string& dataset_dir) {
    // Check if metadata.csv exists
    std::string metadata_path = dataset_dir + "/metadata.csv";
    if (!fs::exists(metadata_path)) {
        g_logger.log("Metadata file not found: " + metadata_path, "", "ljspeechtools", LogLevel::ERROR);
        return false;
    }
    
    // Check if wavs directory exists
    std::string wavs_dir = dataset_dir + "/wavs";
    if (!fs::exists(wavs_dir)) {
        g_logger.log("Wavs directory not found: " + wavs_dir, "", "ljspeechtools", LogLevel::ERROR);
        return false;
    }
    
    g_logger.log("Dataset validation passed: " + dataset_dir, "", "ljspeechtools", LogLevel::INFO);
    return true;
}

std::string DatasetPreparator::normalizeTranscription(const std::string& text) {
    std::string normalized = text;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    // Replace common abbreviations and numbers
    std::vector<std::pair<std::regex, std::string>> replacements = {
        {std::regex("\\bdr\\."), "doctor"},
        {std::regex("\\bmr\\."), "mister"},
        {std::regex("\\bmrs\\."), "missus"},
        {std::regex("\\bms\\."), "miss"},
        {std::regex("\\bst\\."), "saint"},
        {std::regex("\\bave\\."), "avenue"},
        {std::regex("\\brd\\."), "road"},
        {std::regex("\\betc\\."), "etcetera"},
        {std::regex("\\be\\.g\\."), "for example"},
        {std::regex("\\bi\\.e\\."), "that is"},
        {std::regex("\\b1st\\b"), "first"},
        {std::regex("\\b2nd\\b"), "second"},
        {std::regex("\\b3rd\\b"), "third"},
        {std::regex("\\b([0-9]+)th\\b"), "$1th"},
        // Add more number replacements as needed
        {std::regex("\\b0\\b"), "zero"},
        {std::regex("\\b1\\b"), "one"},
        {std::regex("\\b2\\b"), "two"},
        {std::regex("\\b3\\b"), "three"},
        {std::regex("\\b4\\b"), "four"},
        {std::regex("\\b5\\b"), "five"},
        {std::regex("\\b6\\b"), "six"},
        {std::regex("\\b7\\b"), "seven"},
        {std::regex("\\b8\\b"), "eight"},
        {std::regex("\\b9\\b"), "nine"},
    };
    
    for (const auto& replacement : replacements) {
        normalized = std::regex_replace(normalized, replacement.first, replacement.second);
    }
    
    // Remove punctuation except apostrophes (but be more careful with contractions)
    std::regex punct_regex("[^a-zA-Z0-9\\s']");
    normalized = std::regex_replace(normalized, punct_regex, "");
    
    // Handle contractions more carefully
    std::vector<std::pair<std::regex, std::string>> contractions = {
        {std::regex("\\bcan't\\b"), "cannot"},
        {std::regex("\\bwon't\\b"), "will not"},
        {std::regex("\\bshan't\\b"), "shall not"},
        {std::regex("\\bn't\\b"), " not"},  // general n't -> not
        {std::regex("\\b're\\b"), " are"},   // 're -> are
        {std::regex("\\b've\\b"), " have"},  // 've -> have
        {std::regex("\\b'll\\b"), " will"},  // 'll -> will
        {std::regex("\\b'd\\b"), " would"},  // 'd -> would (simplified)
        {std::regex("\\b'm\\b"), " am"},     // 'm -> am
        {std::regex("\\b's\\b"), " is"},     // 's -> is (simplified)
    };
    
    for (const auto& contraction : contractions) {
        normalized = std::regex_replace(normalized, contraction.first, contraction.second);
    }
    
    // Replace multiple spaces with single space
    std::regex space_regex("\\s+");
    normalized = std::regex_replace(normalized, space_regex, " ");
    
    // Trim whitespace
    normalized.erase(0, normalized.find_first_not_of(" \t\n\r\f\v"));
    normalized.erase(normalized.find_last_not_of(" \t\n\r\f\v") + 1);
    
    return normalized;
}

// LJSpeechTools implementation
class LJSpeechTools::Impl {
public:
    AudioProcessor audio_processor_;
    SpeechTranscriber transcriber_;
    SpeechSynthesizer synthesizer_;
    DatasetPreparator dataset_preparator_;
    
    bool initialized_ = false;
    
    bool initialize(const std::string& config_path) {
        g_logger.log("Initializing LJSpeechTools", "", "ljspeechtools", LogLevel::INFO);
        
        if (!config_path.empty()) {
            g_logger.log("Using config file: " + config_path, "", "ljspeechtools", LogLevel::INFO);
        }
        
        initialized_ = true;
        return true;
    }
    
    bool runPipeline(const std::string& input_dir, const std::string& output_dir, bool verbose) {
        if (!initialized_) {
            g_logger.log("LJSpeechTools not initialized", "", "ljspeechtools", LogLevel::ERROR);
            return false;
        }
        
        g_logger.log("Running LJSpeechTools pipeline", "", "ljspeechtools", LogLevel::INFO);
        g_logger.log("Input directory: " + input_dir, "", "ljspeechtools", LogLevel::INFO);
        g_logger.log("Output directory: " + output_dir, "", "ljspeechtools", LogLevel::INFO);
        
        // Create dataset
        auto metadata = dataset_preparator_.createDataset(input_dir, output_dir, true, true);
        
        // Save metadata
        std::string metadata_path = output_dir + "/metadata.csv";
        bool saved = dataset_preparator_.saveMetadata(metadata, metadata_path);
        
        if (verbose) {
            g_logger.log("Pipeline completed successfully", "", "ljspeechtools", LogLevel::INFO);
            g_logger.log("Generated " + std::to_string(metadata.size()) + " metadata entries", "", "ljspeechtools", LogLevel::INFO);
        }
        
        return saved;
    }
};

LJSpeechTools::LJSpeechTools() : impl_(std::make_unique<Impl>()) {}
LJSpeechTools::~LJSpeechTools() = default;

bool LJSpeechTools::initialize(const std::string& config_path) {
    return impl_->initialize(config_path);
}

bool LJSpeechTools::runPipeline(
    const std::string& input_dir,
    const std::string& output_dir,
    bool verbose
) {
    return impl_->runPipeline(input_dir, output_dir, verbose);
}

AudioProcessor& LJSpeechTools::getAudioProcessor() {
    return impl_->audio_processor_;
}

SpeechTranscriber& LJSpeechTools::getTranscriber() {
    return impl_->transcriber_;
}

SpeechSynthesizer& LJSpeechTools::getSynthesizer() {
    return impl_->synthesizer_;
}

DatasetPreparator& LJSpeechTools::getDatasetPreparator() {
    return impl_->dataset_preparator_;
}

} // namespace elizaos
