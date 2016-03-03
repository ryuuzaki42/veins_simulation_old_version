## Script in R ##
## Execute R < script_r.r or source('script_r.r', echo=TRUE)


calculeDistance <- function(){
  ## copy in a=c("values") and b=c("values")
  a=c(746.373,523.35,0)
  b=c(523.35,519.12,0)
  sqrt((a[1]-b[1])^2 + (a[2] - b[2])^2)
}

calculeDistance()