import json
import os

dict_file = r'c:\Users\hp\OneDrive\Desktop\PYTHON\AutoCorrect_PredictiveText\data\dictionary.txt'

# Comprehensive WhatsApp Shortcuts / Slang Dictionary
whatsapp_slang = {
    'idk': "I don't know",
    'brb': "Be right back",
    'tbh': "To be honest",
    'imo': "In my opinion",
    'imho': "In my humble opinion",
    'btw': "By the way",
    'rn': "Right now",
    'np': "No problem",
    'ttyl': "Talk to you later",
    'tysm': "Thank you so much",
    'ty': "Thank you",
    'thx': "Thank you",
    'pls': "Please",
    'plz': "Please",
    'wbu': "What about you?",
    'hru': "How are you?",
    'gm': "Good morning",
    'gn': "Good night",
    'tc': "Take care",
    'afaik': "As far as I know",
    'ofc': "Of course",
    'nvm': "Never mind",
    'wyd': "What are you doing?",
    'wfh': "Work from home",
    'omg': "Oh my god",
    'gtg': "Got to go",
    'fyi': "For your information",
    'asap': "As soon as possible",
    'dm': "Direct message",
    'aka': "Also known as",
    'diy': "Do it yourself",
    'pov': "Point of view",
    'gg': "Good game",
    'gl': "Good luck",
    'hf': "Have fun",
    'bbl': "Be back later"
}

contractions_and_proper = {
    'i': 'I',
    'im': "I'm",
    "i'm": "I'm",
    'ill': "I'll",
    "i'll": "I'll",
    'ive': "I've",
    "i've": "I've",
    'id': "I'd",
    "i'd": "I'd",
    'hows': "how's",
    "how's": "how's",
    'lets': "let's",
    "let's": "let's",
    'whats': "what's",
    "what's": "what's",
    'thats': "that's",
    "that's": "that's",
    'theres': "there's",
    "there's": "there's",
    'heres': "here's",
    "here's": "here's",
    'wheres': "where's",
    "where's": "where's",
    'whens': "when's",
    "when's": "when's",
    'whys': "why's",
    "why's": "why's",
    'whos': "who's",
    "who's": "who's",
    'its': "it's",
    "it's": "it's",
    'youre': "you're",
    "you're": "you're",
    'theyre': "they're",
    "they're": "they're",
    'were': "we're",
    "we're": "we're",
    'dont': "don't",
    "don't": "don't",
    'cant': "can't",
    "can't": "can't",
    'wont': "won't",
    "won't": "won't",
    'didnt': "didn't",
    "didn't": "didn't",
    'doesnt': "doesn't",
    "does't": "doesn't",
    'isnt': "isn't",
    "isn't": "isn't",
    'arent': "aren't",
    "aren't": "aren't",
    'wasnt': "wasn't",
    "wasn't": "wasn't",
    'werent': "weren't",
    "weren't": "weren't",
    'havent': "haven't",
    "haven't": "haven't",
    'hasnt': "hasn't",
    "hasn't": "hasn't",
    'hadnt': "hadn't",
    "hadn't": "hadn't",
    'couldnt': "couldn't",
    "couldn't": "couldn't",
    'wouldnt': "wouldn't",
    "wouldn't": "wouldn't",
    'shouldnt': "shouldn't",
    "shouldn't": "shouldn't",
    'mustnt': "mustn't",
    "mustn't": "mustn't",
    'india': 'India',
    'bharat': 'Bharat',
    'delhi': 'Delhi',
    'mumbai': 'Mumbai',
    'bangalore': 'Bangalore',
    'python': 'Python',
    'javascript': 'JavaScript',
    'c': 'C',
    'cpp': 'C++'
}

freq_overrides = {
    'I': 10000,
    "I'm": 9950,
    "I'll": 9500,
    "I've": 9400,
    "I'd": 9200,
    "how's": 9800,
    "let's": 9800,
    "what's": 9750,
    "that's": 9600,
    "there's": 9500,
    "here's": 9400,
    "where's": 9400,
    "who's": 9400,
    "it's": 9850,
    "don't": 9700,
    "can't": 9600,
    "you're": 9500,
    "we're": 9400,
    "they're": 9400,
    'India': 9900,
    'Bharat': 9200
}

word_dict = {}
if os.path.exists(dict_file):
    with open(dict_file, 'r', encoding='utf-8') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                w_raw = parts[0].lower().replace("'", "")
                freq = int(parts[1])
                disp = contractions_and_proper.get(w_raw, parts[0])
                word_dict[disp] = max(word_dict.get(disp, 0), freq)

# Add overrides
for disp_word, f_val in freq_overrides.items():
    word_dict[disp_word] = f_val

for raw_k, disp_k in contractions_and_proper.items():
    if disp_k not in word_dict:
        word_dict[disp_k] = 9200

# Add slang entries to dictionary with high priority
for slang_k, slang_expansion in whatsapp_slang.items():
    word_dict[slang_expansion] = 9850

sorted_words = sorted(word_dict.items(), key=lambda x: (-x[1], x[0]))

# Update dictionary.txt
with open(dict_file, 'w', encoding='utf-8') as f:
    for w, f_val in sorted_words:
        f.write(f'{w} {f_val}\n')

# Update dict_data.js with both word dictionary and WHATSAPP_SLANG mapping
js_file = r'c:\Users\hp\OneDrive\Desktop\PYTHON\AutoCorrect_PredictiveText\web\dict_data.js'
with open(js_file, 'w', encoding='utf-8') as f:
    f.write('// Auto-generated dictionary with WhatsApp Shortcuts, Contractions & Proper Nouns\n')
    f.write('window.WHATSAPP_SLANG = ' + json.dumps(whatsapp_slang) + ';\n')
    f.write('window.DEFAULT_DICTIONARY = ' + json.dumps([[w, f_val] for w, f_val in sorted_words]) + ';\n')

print(f'Successfully updated dictionary with {len(sorted_words)} entries + {len(whatsapp_slang)} WhatsApp shortcuts!')
