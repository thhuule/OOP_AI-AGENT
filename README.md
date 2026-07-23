# OOP_AI-AGENT
**GG Colab Client: https://colab.research.google.com/drive/1J41KxBCeKqclHm-n0WRzQi8fRKKKKdtO?usp=sharing**


**bash test gemini api:** 
curl -s "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent" \
  -H 'Content-Type: application/json' \
  -H 'x-goog-api-key: "YOUR_API_KEY"' \
  -X POST \
  -d '{"contents": [{"parts": [{"text": "Hello Gemini!"}]}]}' | jq '.candidates[0].content.parts[0].text'