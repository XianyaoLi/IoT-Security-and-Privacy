1)	Create Self-Signed Certificate to act as Certificate Authority:
a)	Create Certificate Authority
	openssl req -nodes -x509 -newkey rsa:2048 -keyout CA.key.pem -out CA.crt.pem -sha256 -days 365
Note: The Common Name (CN) must match your computer’s IP address. I recommend using localhost for this lab
a)	Verify Certificate Authority:
openssl x509 -noout -text -in ca.crt.pem
b)	Verify Certificate matches Key:
openssl rsa -noout -modulus -in ca.key.pem | openssl sha256
openssl x509 -noout -modulus -in ca.crt.pem | openssl sha256

2)	Create and Sign a Certificate:
a)	Create a Private RSA Key:
openssl genrsa -out client.key.pem 2048
b)	Create Certificate Signing Request:
	openssl req -new -key client.key.pem -out client.csr

Note: each certificate needs some field filled differently in the CSR (I recommend different emails, e.g., root@ca, broker@ca, etc.)
Note: The Common Name (CN) must match your computer’s IP address. I recommend using localhost for this lab
c)	Create a Certificate using the CSR:
openssl x509 -req -days 365 -in client.csr -CA CA.crt.pem -CAkey CA.key.pem -CAcreateserial -out client.crt.pem
d)	Verify Certificate:
openssl x509 -noout -text -in client.crt.pem
e)	Verify Certificate matches Key:
openssl rsa -noout -modulus -in client.key.pem | openssl sha256
openssl x509 -noout -modulus -in client.crt.pem | openssl sha256


