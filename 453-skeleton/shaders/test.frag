#version 330 core

in vec3 fragPos;
in vec3 n;
in vec2 tc;


uniform vec3 objectPos;
uniform float radius;
uniform sampler2D sampler0;
uniform sampler2D sampler1;  //night
uniform sampler2D sampler2;	 //clouds
uniform sampler2D sampler3;	 //bump
uniform float shade;
uniform vec3 camPos;


out vec4 color;


//// lightCol is for are specular
//// ambient is for the ambient light
//// diff is for diffuse
//// d3.r is the bump mapping. higher r = higher elevation not sure if implemented correctly honestly.

void main() {
	//moon
	if (shade == 1){

		//difuse
		vec3 lightDir = normalize(-1 * fragPos);
		vec3 normal = normalize(n);
		float diff = max(dot(lightDir, normal), 0.0);
		float x = 0;
		float z = 0;


		//specular
		vec3 reflection = normalize(reflect(lightDir, normal));
		vec3 toCamDir = normalize(fragPos - camPos);
		float alpha = pow(max(dot(toCamDir,reflection), 0), 4);
		vec4 lightCol = vec4(0.5,0.5,0.5,1) * alpha;


		//earth blocking sun
		for (float i =0; i <= 1; i += 0.1){
			x = fragPos.x * i;
			z = fragPos.z * i;
			if( sqrt(pow(x-objectPos.x, 2)  + pow(z - objectPos.z, 2) ) <= radius ){
				diff = 0;
				lightCol = vec4(0);
				break;
			}
		}

		//ambient
		float ambient = 0.1;

		vec4 d = texture(sampler0, tc);
		color = vec4(diff * d + d * ambient  + lightCol);
		
	}
	//earth
	else if (shade == 2){
		vec3 lightDir = normalize(-1 * fragPos);
		vec3 normal = normalize(n);
		float diff = max(dot(lightDir, normal), 0.0);


		//night side
		if(diff == 0) {
			vec4 d1 = texture(sampler1, tc);
			vec4 d2 = texture(sampler2, tc);
			float ambient = 0.3;
			color = d1 * ambient;
		}
		//day side
		else{
				//specular
				vec3 reflection = normalize(reflect(lightDir, normal));
				vec3 toCamDir = normalize(fragPos - camPos);
				float alpha = pow(max(dot(toCamDir,reflection), 0), 7);
				vec4 lightCol = vec4(0.8,0.8,0.8,1) * alpha;

				//ambient
				float ambient = 0.1;

				bool dark = false;
				float x = 0;
				float z = 0;
				for (float i =0; i <= 1; i += 0.04){
					x = fragPos.x * i;
					z = fragPos.z * i;
					if( sqrt(pow(x-objectPos.x, 2)  + pow(z - objectPos.z, 2) ) <= radius ){
						diff = 0;
						lightCol = vec4(0);
						dark = true;
						break;
					}
				}

				vec4 d = texture(sampler0, tc);
				d.a=0.3;
				vec4 d3 = texture(sampler3, tc);


				//shiny ocean
				if(d.b > d.r && d.b > d.g){
					alpha = pow(max(dot(toCamDir,reflection), 0), 1);
					vec4 lightCol = vec4(0.6,0.6,1, 1) * alpha;
					if(dark){
						color = (d * ambient);
					}
					else{
						color = vec4(diff * d + d * ambient + lightCol);
					}
				}
				//bump mapping
				else{
					if(dark){
						color = vec4(d*ambient);
					}
					color = vec4(diff * d + d * ambient + lightCol * d3.r);
				}
		}
	}
	//clouds
	else if (shade == 3){

		vec4 d2 = texture(sampler2, tc);
		if(d2.r < 0.05){
			discard;
		}

		//difuse
		vec3 lightDir = normalize(-1 * fragPos);
		vec3 normal = normalize(n);
		float diff = max(dot(lightDir, normal), 0.0);

		//night side ambient
		if(diff == 0) {
			d2.a = 0.4;
			//discard;					//uncomment me
			color = d2 * 0.1;
		}
		//day side
		else{
			//specular
			vec3 reflection = normalize(reflect(lightDir, normal));
			vec3 toCamDir = normalize(fragPos - camPos);
			float alpha = pow(max(dot(toCamDir,reflection), 0), 3);
			vec4 lightCol = vec4(0.5,0.5,0.5,0.5) * alpha;

			//ambient
			float ambient = 0.1;


			float x = 0;
			float z = 0;
			for (float i = 0; i <= 1; i += 0.05){
				x = fragPos.x * i;
				z = fragPos.z * i;
				if( sqrt(pow(x-objectPos.x, 2)  + pow(z - objectPos.z, 2) ) <= radius ){
					diff = 0;
					lightCol = vec4(0);
				}
			}
			//d2.a = 1;
			color = diff * d2  + d2 * ambient + lightCol;
		}
	}
	//sun and stars
	else{
		vec4 d = texture(sampler0, tc);
		color = vec4(d);
	}
}


