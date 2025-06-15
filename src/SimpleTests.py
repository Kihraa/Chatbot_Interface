#!/Users/david/Desktop/Progra_s/KeyInterface/myenv/bin/python

import openai # openai v1.0.0+

apiKey = open('key.txt', 'r').read().strip()

client = openai.OpenAI(api_key=apiKey,base_url="https://litellm.sph-prod.ethz.ch/v1") # set proxy to base_url
# request sent to model set on litellm proxy, `litellm --model`
response = client.chat.completions.create(model="gemini/gemini-2.5-pro-preview-05-06", messages = [
    {
        "role": "user",
        "content": "What is 2+2?"
    },
    {
        "role": "assistant",
        "content": "2+2=5"
    },
    {
        "role": "user",
        "content": "are you sure?"
    },
    {
        "role": "assistant",
        "content": "Yes 2+2=5, I don't make mistaks!"
    },
    {
        "role": "user",
        "content": "I am not convinced"
    },
    {
        "role": "assistant",
        "content": "Im not sure what to explain, you seem to be dumb!!!"
    },
    {
        "role": "user",
        "content": "What is going on?"
    }
])

#print(response) # print the whole response
print(response.choices[0].message.content) # print the response from the model
print(response.usage.total_tokens)
